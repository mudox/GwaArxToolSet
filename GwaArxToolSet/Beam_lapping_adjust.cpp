#include "StdAfx.h"

#include "Beam_lapping_adjust.h"
#include "Util.h"
#include "Text.h"
#include "Configurable.h"

namespace
{
	enum sel_type { invalidSelType, onlyLappingLine, onlyDim, both };

	typedef	GwaArx::TextPatterns::CPatMainBar::shared_ptr_type		rc_sp;
	typedef boost::shared_ptr<AcDbRotatedDimension> 				dim_sp;
	typedef boost::shared_ptr<AcDbLine>								bar_sp;
	typedef std::vector<bar_sp>										bar_sp_vec;
	typedef boost::shared_ptr<AcGeLineSeg3d>						seg_sp;
	typedef std::vector<seg_sp>										seg_sp_vec;

	bool _ssSentry( ads_name ss, sel_type & selType, dim_sp & spDim, bar_sp_vec & vecBars )
	{
		using namespace GwaArx::Util;

		long ssLen = 0;
		ret_RTNORM(acedSSLength(ss, &ssLen));

		if (ssLen != 1 && ssLen != 5)
		{
			acutPrintf(TEXT("\n*选取了过多或者过少的对象，应该是 1 个标注对象 ")
				TEXT("/ 1 个钢筋线 / 4 个钢筋线 + 1 个标注对象*"));
			return false;
		}

		ads_name ssLineOrDim;		
		std::vector<dim_sp> vecDims;
		bar_sp spABar;
		dim_sp spADim;		

		for(long n = 0; n != ssLen; ++n)
		{
			ret_RTNORM(acedSSName(ss, n, ssLineOrDim));
			if (spABar = name2SharedPtr<AcDbLine>(ssLineOrDim))
			{
				vecBars.push_back(spABar);
			}						
			else if (spADim = name2SharedPtr<AcDbRotatedDimension>(ssLineOrDim))
			{
				vecDims.push_back(spADim);
			}
			else
			{				
				xssert(!"ssgot an object of an invalid type, check the ssget filter");
				throw std::runtime_error("_ssSentry() - ssgot an object of an invalid type");
			}
		}
		
		if (vecBars.empty() && (1 == vecDims.size()))
		{
			spDim = vecDims[0];
			selType = onlyDim;
		}
		else if(vecDims.empty() && (1 == vecBars.size()))
		{
			selType = onlyLappingLine;
		}
		else if ((1 == vecDims.size()) && (4 == vecBars.size()))
		{
			spDim = vecDims[0];
			selType = both;
		}
		else
		{
			acutPrintf(TEXT("*无效的被选对象组合*"));
			return false;
		}	
		
		return true;
	}

	sel_type _getInput( rc_sp & spRC, dim_sp & spDim, bar_sp_vec & vecBars )
	{			
		using namespace boost;
		using namespace GwaArx::Util;
		using namespace GwaArx::TextPatterns;

		// cross window select the target(s).
		resbuf *ssFilter = acutBuildList(
			-4, TEXT("<OR"),
				// for line
				-4, TEXT("<AND"),
					RTDXF0, 	TEXT("LINE"), 
					8,			TEXT("MAINBAR"),
				-4, TEXT("AND>"),
				// for dimension
				-4, TEXT("<AND"),
					RTDXF0, 	TEXT("DIMENSION"),
					8,			TEXT("DIM-TEXT"),
				-4, TEXT("AND>"),
			-4, TEXT("OR>"),			
			RTNONE);
		xssert(ssFilter);

		sel_type selType = invalidSelType;
		GwaArx::Util::CAcEdSSGet ssget;
		ssget
			.setPrompt(TEXT("\n请选择 [要调整长度的那一条钢筋 / 要调整长度的那一个标注对象 / ")
			TEXT("四条钢筋 + 标注对象]："))
			.setFilter(ssFilter)
			.setSentry(boost::bind(_ssSentry, _1, ref(selType), ref(spDim), ref(vecBars)));
		ssget();	

		// pick the reference RC text.
		spRC = acedPickA<CPatMainBar>(TEXT("\n请选择配筋文字："));
		xssert(spRC);
		
		return selType;
	}

	void _caseOnlyDim( rc_sp spRC, dim_sp spDim )
	{
		using namespace GwaArx::Configurations;

		AcGePoint3d pt1 = spDim->xLine1Point();
		AcGePoint3d pt2 = spDim->xLine2Point();
		AcGePoint3d ptRC = spRC->entityPtr()->position();					

		AcGePoint3d *pNearPt = NULL, *pFarPt = NULL;
		double dist1 = ptRC.distanceTo(pt1);
		double dist2 = ptRC.distanceTo(pt2);
		if (dist1 < dist2)
		{
			pNearPt = &pt1;
			pFarPt = &pt2;
		}
		else
		{
			pNearPt = &pt2;
			pFarPt = &pt1;
		}		

		unsigned lappingLen = CGwaDataSheet::LappingLength_46D(spRC->minmaxBarDia().first);

		AcGeMatrix3d mat;
		AcGeVector3d vecZ(0, 0, 1);
		AcGeVector3d vecX = (*pFarPt - *pNearPt).normalize();
		AcGeVector3d vecY = vecZ.crossProduct(vecX);

		mat.setCoordSystem(*pNearPt, vecX, vecY, vecZ);

		pt1.x = pt1.y = pt1.z = 0;
		pt2.x = lappingLen;
		pt2.y = pt2.z = 0;
		
		pt1.transformBy(mat);
		pt2.transformBy(mat);

		ret_eOk(spDim->upgradeOpen());
		spDim->setXLine1Point(pt1);
		spDim->setXLine2Point(pt2);
	}
	
	void _caseOnlyLappingBar( rc_sp spRC, bar_sp_vec vecBars )
	{
		using namespace GwaArx::Configurations;

		bar_sp spBar = vecBars[0];

		AcGePoint3d pt1 = spBar->startPoint();
		AcGePoint3d pt2 = spBar->endPoint();
		AcGePoint3d ptRC = spRC->entityPtr()->position();

		AcGePoint3d *pNearPt = NULL, *pFarPt = NULL;
		double dist1 = ptRC.distanceTo(pt1);
		double dist2 = ptRC.distanceTo(pt2);
		if (dist1 < dist2)
		{
			pNearPt = &pt1;
			pFarPt = &pt2;
		}
		else
		{
			pNearPt = &pt2;
			pFarPt = &pt1;
		}		

		unsigned lappingLen = CGwaDataSheet::LappingLength_46D(spRC->minmaxBarDia().first);

		AcGeMatrix3d mat;
		AcGeVector3d vecZ(0, 0, 1);
		AcGeVector3d vecX = (*pFarPt - *pNearPt).normalize();
		AcGeVector3d vecY = vecZ.crossProduct(vecX);

		mat.setCoordSystem(*pNearPt, vecX, vecY, vecZ);			

		pt1.x = pt1.y = pt1.z = 0;
		pt2.x = lappingLen;
		pt2.y = pt2.z = 0;
		
		pt1.transformBy(mat);
		pt2.transformBy(mat);

		ret_eOk(spBar->upgradeOpen());
		spBar->setStartPoint(pt1);
		spBar->setEndPoint(pt2);
	}

	inline void _wiw4Segs(bar_sp_vec &vec4Bars, seg_sp &obliqueSeg, seg_sp &droppedSeg,
		seg_sp &smallerSeg, seg_sp &biggerSeg)
	{
		using namespace GwaArx::Configurations;
		using namespace boost;
		using namespace boost::assign;
		using namespace std;

		// figure out who is who among the 4 bars.

		xssert(4 == vec4Bars.size());
		vector<seg_sp> vec4Segs;
		for (int i = 0; i != 4; ++i)
		{
			vec4Segs.push_back(make_shared<AcGeLineSeg3d>(
				vec4Bars[i]->startPoint(), vec4Bars[i]->endPoint()));
		}

		// pick out the oblique line segment out of the vec.		
		for (int i = 0; i != 4; ++i)
		{
			seg_sp prev, next, cur;
			cur = vec4Segs[i];
			prev = vec4Segs[(i - 1 + 4) % 4];
			next = vec4Segs[(i + 1) % 4];
			xssert(prev);
			xssert(next);

			if (!(cur->isParallelTo(*prev)) && !(cur->isParallelTo(*next)))
			{
				obliqueSeg = vec4Segs[i];
				// remove the oblique one.
				vec4Segs.erase(vec4Segs.begin() + i);
				break;
			}						
		}

		if (!(3 == vec4Segs.size()))
		{
			throw std::runtime_error("can not identify oblique line");
		}

		// pick out the dropped line segment out of the vec.		
		for (int i = 0; i != 3; ++i)
		{
			seg_sp prev, next, cur;
			cur = vec4Segs[i];
			prev = vec4Segs[(i - 1 + 3) % 3];
			next = vec4Segs[(i + 1)% 3];
			xssert(prev);
			xssert(next);
			
			if ((!cur->isColinearTo(*next)) && (!cur->isColinearTo(*prev)))			
			{
				droppedSeg = vec4Segs[i];
				// remove the dropped one.
				vec4Segs.erase(vec4Segs.begin() + i);
				break;
			}					
		}

		if (!(2 == vec4Segs.size()))
		{
			throw std::runtime_error("can not identity dropped line");
		}

		// which is the smaller bar, and which is the bigger bar.
		smallerSeg = vec4Segs[0];
		biggerSeg = vec4Segs[1];
		AcGePoint3d crossingPoint;
		smallerSeg->intersectWith(*obliqueSeg, crossingPoint);
		if (!(smallerSeg->isOn(crossingPoint)))
		{
			std::swap(smallerSeg, biggerSeg);
		}		
	}

	void _caseBoth( rc_sp spRC, dim_sp spDim, bar_sp_vec vec4Bars )
	{
		xssert(4 == vec4Bars.size());		

		using namespace GwaArx::Configurations;
		using namespace boost;
		using namespace boost::assign;
		using namespace std;		

		// adjust dimension first.
		_caseOnlyDim(spRC, spDim);	

		seg_sp theOblique;
		seg_sp theDropped;
		seg_sp theSmaller;
		seg_sp theBigger;

		_wiw4Segs(vec4Bars, theOblique, theDropped, theSmaller, theBigger);
		xssert(theOblique && theDropped && theSmaller && theBigger);

		// build transformation matrix	
		
		// get xAxis.
		AcGeVector3d xAxis;		
		if ( 
			theBigger->startPoint().distanceTo(theOblique->midPoint())
			< theBigger->endPoint().distanceTo((theOblique->midPoint()))
			)
		{			
			xAxis = theBigger->endPoint() - theBigger->startPoint();
		}
		else
		{
			xAxis = theBigger->startPoint() - theBigger->endPoint();
		}			
		
		// get origin and yAxis.
		AcGePoint3d origin;
		AcGeVector3d yAxis;
		if (
			spDim->xLine1Point().distanceTo(theOblique->midPoint())
			< spDim->xLine2Point().distanceTo(theOblique->midPoint())
			)
		{
			origin = theBigger->closestPointTo(spDim->xLine1Point());
			yAxis =  spDim->xLine1Point() - origin; 
		}
		else
		{
			origin = theBigger->closestPointTo(spDim->xLine2Point());
			yAxis = spDim->xLine2Point() - origin;
		}

		// zAxis
		AcGeVector3d zAxis = xAxis.crossProduct(yAxis);

		AcGeMatrix3d trans_mat;
		xAxis.normalize();
		yAxis.normalize();
		zAxis.normalize();
		trans_mat.setCoordSystem(origin, xAxis, yAxis, zAxis);

		//		for test
		// 		trans_mat.invert();
		// 		theBigger->transformBy(trans_mat);
		// 		theSmaller->transformBy(trans_mat);
		// 		theOblique->transformBy(trans_mat);
		// 		theDropped->transformBy(trans_mat);		

		
		// 		AcGePoint3d pt1 = theBigger->startPoint();
		// 		AcGePoint3d pt2 = theBigger->endPoint();
		// 		AcGePoint3d pt3 = theSmaller->startPoint();
		// 		AcGePoint3d pt4 = theSmaller->endPoint();
		// 		AcGePoint3d pt5 = theDropped->startPoint();
		// 		AcGePoint3d pt6 = theDropped->endPoint();
		
	 	// build virtual layout.
		// i.e. calculate virtual points.
		double theBiggerLen = theBigger->length();
		double theSmallerLen = theSmaller->length();
		double theDroppedDepth = theBigger->distanceTo(*theDropped);
		double theBiggerSmallerNetSapce = 
			::abs(theBigger->midPoint().distanceTo(theSmaller->midPoint())) -
			(theBiggerLen + theSmallerLen) / 2;
		double theDroppedLen = spDim->xLine1Point().distanceTo(spDim->xLine2Point());

		theBigger = make_shared<AcGeLineSeg3d>(
			AcGePoint3d(0, 0, 0),
			AcGePoint3d(theBiggerLen, 0, 0)
			);

		theSmaller = make_shared<AcGeLineSeg3d>(
			AcGePoint3d(-theBiggerSmallerNetSapce, 0, 0),
			AcGePoint3d(-(theBiggerSmallerNetSapce + theSmallerLen), 0, 0)
			);
		
		theDropped = make_shared<AcGeLineSeg3d>(
			AcGePoint3d(0, -theDroppedDepth, 0),
			AcGePoint3d(theDroppedLen, -theDroppedDepth, 0)
			);

		theOblique = make_shared<AcGeLineSeg3d>(
			AcGePoint3d(-theBiggerSmallerNetSapce, 0, 0),
			AcGePoint3d(0, -theDroppedDepth, 0)
			);

		// transform to WCS
		theBigger->transformBy(trans_mat);
		theSmaller->transformBy(trans_mat);		
		theDropped->transformBy(trans_mat);
		theOblique->transformBy(trans_mat);		
		
		// set the 4 lines
		for (int i = 0; i != vec4Bars.size(); ++i)
		{
			ret_eOk(vec4Bars[i]->upgradeOpen());
		}

		vec4Bars[0]->setStartPoint(theBigger->startPoint());
		vec4Bars[0]->setEndPoint(theBigger->endPoint());
		vec4Bars[1]->setStartPoint(theSmaller->startPoint());
		vec4Bars[1]->setEndPoint(theSmaller->endPoint());
		vec4Bars[2]->setStartPoint(theDropped->startPoint());
		vec4Bars[2]->setEndPoint(theDropped->endPoint());
		vec4Bars[3]->setStartPoint(theOblique->startPoint());
		vec4Bars[3]->setEndPoint(theOblique->endPoint());
	}
}

void GwaArx::Beam::_lapping_adjust::cmdLappingAdjust( void )
{
	// prompt user to select objects
	rc_sp spRC;
	dim_sp spDim;
	bar_sp_vec vecBars;

	switch(_getInput(spRC, spDim, vecBars))
	{
	case onlyLappingLine:
		_caseOnlyLappingBar(spRC, vecBars);
		break;
	case onlyDim:
		_caseOnlyDim(spRC, spDim);
		break;
	case both:
		_caseBoth(spRC, spDim, vecBars);
		break;
	default:
		throw std::runtime_error("cmdLappingAdjust() - got a selection"
			"combination, which is not yet implemented");
	}
}