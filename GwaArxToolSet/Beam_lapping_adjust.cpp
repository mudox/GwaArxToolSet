#include "StdAfx.h"

#include "Beam_lapping_adjust.h"
#include "Util.h"
#include "Text.h"
#include "Configurable.h"

namespace
{
	enum sel_type { invalidSelType, onlyLappingLine, onlyDim, both };

	typedef	GwaArx::TextPatterns::CPatMainBar::shared_ptr_type rc_sp;
	typedef boost::shared_ptr<AcDbRotatedDimension> dim_sp;
	typedef boost::shared_ptr<AcDbLine> bar_sp;
	typedef std::vector<bar_sp> bar_sp_vec;

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

		mat.invert();
 		pt1.transformBy(mat);
 		pt2.transformBy(mat);		

		pFarPt->x = lappingLen;

		mat.invert();
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

		mat.invert();
		pt1.transformBy(mat);
		pt2.transformBy(mat);		

		pFarPt->x = lappingLen;

		mat.invert();
		pt1.transformBy(mat);
		pt2.transformBy(mat);

		ret_eOk(spBar->upgradeOpen());
		spBar->setStartPoint(pt1);
		spBar->setEndPoint(pt2);
	}
	
	void _caseBoth( rc_sp spRC, dim_sp spDim, bar_sp_vec vecBars )
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

		mat.invert();
		pt1.transformBy(mat);
		pt2.transformBy(mat);		

		pFarPt->x = lappingLen;

		mat.invert();
		pt1.transformBy(mat);
		pt2.transformBy(mat);

		ret_eOk(spDim->upgradeOpen());
		spDim->setXLine1Point(pt1);
		spDim->setXLine2Point(pt2);
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