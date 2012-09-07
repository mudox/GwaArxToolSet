#include "StdAfx.h"

#include "Beam_lapping_adjust.h"
#include "Util.h"
#include "Text.h"
#include "Configurable.h"


namespace
{
	boost::shared_ptr<AcDbRotatedDimension> _dimSentry( AcDbObjectId id )
	{
		return GwaArx::Util::id2SharedPtr<AcDbRotatedDimension>(
			id, AcDb::kForWrite);
	}

	bool _lowerY(boost::shared_ptr<AcDbLine> lhs, boost::shared_ptr<AcDbLine> rhs)
	{
		return lhs->startPoint().y < rhs->startPoint().y;
	}

	class CGwaLine
	{
	public:
		typedef	boost::shared_ptr<AcDbLine> line_sp_type;
		typedef BOOST_TYPEOF(boost::bind(&AcDbLine::setStartPoint, (AcDbLine *)0, _1)) setter_type;

		CGwaLine(line_sp_type sp)
			: m_theLine(sp)
		{
			xssert(sp);		
		}

	public:
		setter_type ucsLeftEndSettter()
		{
			using namespace GwaArx::Util;

			AcGePoint3d p3dStart = m_theLine->startPoint();
			AcGePoint3d p3dEnd = m_theLine->endPoint();
			
			acdbWcs2Ucs(asAdsPoint(p3dStart), asAdsPoint(p3dStart), false);
			acdbWcs2Ucs(asAdsPoint(p3dEnd), asAdsPoint(p3dEnd), false);

			double xStart = p3dStart.x;
			double xEnd = p3dEnd.x;
			
			return bind(
				((xStart < xEnd) ? (&AcDbLine::setStartPoint) : (&AcDbLine::setEndPoint)),
				m_theLine.get(), 
				_1
				);
		}

		setter_type ucsRightEndSetter()
		{
			using namespace GwaArx::Util;

			AcGePoint3d p3dStart = m_theLine->startPoint();
			AcGePoint3d p3dEnd = m_theLine->endPoint();

			acdbWcs2Ucs(asAdsPoint(p3dStart), asAdsPoint(p3dStart), false);
			acdbWcs2Ucs(asAdsPoint(p3dEnd), asAdsPoint(p3dEnd), false);

			double xStart = p3dStart.x;
			double xEnd = p3dEnd.x;

			return bind(
				((xStart > xEnd) ? (&AcDbLine::setStartPoint) : (&AcDbLine::setEndPoint)),
				m_theLine.get(), 
				_1
				);
		}

		void setUcsLeftPoint( const AcGePoint3d & p3d ) 
		{
			ucsLeftEndSettter()(p3d);
		}

		void setUcsRightPoint(const AcGePoint3d & p3d )
		{
			ucsRightEndSetter()(p3d);
		}
		
		AcGePoint3d ucsLeftPoint( void )
		{
			using namespace GwaArx::Util;

			AcGePoint3d p3dStart = m_theLine->startPoint();
			AcGePoint3d p3dEnd = m_theLine->endPoint();

			acdbWcs2Ucs(asAdsPoint(p3dStart), asAdsPoint(p3dStart), false);
			acdbWcs2Ucs(asAdsPoint(p3dEnd), asAdsPoint(p3dEnd), false);

			double xStart = p3dStart.x;
			double xEnd = p3dEnd.x;

			return (xStart < xEnd) ? p3dStart : p3dEnd;
		}

		AcGePoint3d ucsRightPoint( void )
		{
			using namespace GwaArx::Util;

			AcGePoint3d p3dStart = m_theLine->startPoint();
			AcGePoint3d p3dEnd = m_theLine->endPoint();

			acdbWcs2Ucs(asAdsPoint(p3dStart), asAdsPoint(p3dStart), false);
			acdbWcs2Ucs(asAdsPoint(p3dEnd), asAdsPoint(p3dEnd), false);

			double xStart = p3dStart.x;
			double xEnd = p3dEnd.x;

			return (xStart > xEnd) ? p3dStart : p3dEnd;
		}
				

	private:
		line_sp_type m_theLine;
	};
}

//void GwaArx::Beam::_lapping_adjust::cmdLappingAdjust_old( void )
//{
//	using namespace GwaArx::Util;
//	using namespace GwaArx::TextPatterns;
//	using namespace GwaArx::Configurations;
//	using namespace boost;
//
//	// pick a the dimension object on the movable ends.	
//	BOOST_AUTO(spDim, acedPick(
//		bind(&id2SharedPtr<AcDbRotatedDimension>, _1, AcDb::kForWrite, false), 
//		TEXT("\n请点取搭接标注：")));
//
//	// cross window select the bar line if any.
//	resbuf *ssFilter = acutBuildList(
//		RTDXF0, TEXT("LINE"), 
//		8, TEXT("MAINBAR"),
//		RTNONE);
//	xssert(ssFilter);
//
//	ads_name ss4BarLines;
//	long ssLen = 0;
//
//	acutPrintf(TEXT("\n请选择 4 条搭接钢筋线："));
//	switch(acedSSGet(NULL, NULL, NULL, ssFilter, ss4BarLines))
//	{
//	case RTNORM:
//		ret_RTNORM(acedSSLength(ss4BarLines, &ssLen));		
//		break;
//	case RTERROR:
//		// user pressed space or enter
//		// leaves ssLen as 0;
//		break;
//	case RTCAN:
//		throw UserCanceled();
//	default:
//		;
//	}
//
//	// pick the reference RC text.
//	BOOST_AUTO(pRC,	acedPickA<CPatMainBar>(TEXT("\n请选择配筋文字：")));
//	xssert(pRC);
//
//
//	// adjust the dimension length.
//	// adjust the end of dimension object which is father from the reference 
//	// text object.
//	unsigned lapLen = CGwaDataSheet::LappingLength_46D(pRC->minmaxBarDia().second);
// 	AcGePoint3d xPt1 = spDim->xLine1Point();
// 	AcGePoint3d xPt2 = spDim->xLine2Point();
//
//	BOOST_AUTO(p3d, pRC->entityPtr()->position());
//	bool bRet = acdbWcs2Ucs(asAdsPoint(p3d), asAdsPoint(p3d), false);
//	xssert(bRet);
//	double xRC = p3d.x;
//	
//	double dist1 = std::abs(xPt1.x - xRC);
// 	double dist2 = std::abs(xPt2.x - xRC);
//
//	bool bXLine1PointFixed = true;
//	if (dist1 > dist2)	// xLine1Point is farther, move xPt1
//	{		
//		bXLine1PointFixed = false;
//		xPt1.x = (xPt1.x < xPt2.x) ? (xPt2.x - lapLen) : (xPt2.x + lapLen);
//		spDim->setXLine1Point(xPt1);		
//	}
//	else				// xLine2Point is near, move xPt2
//	{		
//		xPt2.x = (xPt2.x < xPt1.x) ? (xPt1.x - lapLen) : (xPt1.x + lapLen);
//		spDim->setXLine2Point(xPt2);	
//	}
//
//	// adjust the lapping bars, if got a group of 4 valid lines.
//	typedef shared_ptr<AcDbLine> sp_t;
//	typedef std::set<sp_t> set_t;
//	if (4 == ssLen)
//	{
//		set_t setLines;
//
//		for (int n = 0; n != 4; ++n)
//		{
//			ads_name line;
//			acedSSName(ss4BarLines, n, line);
//			setLines.insert(name2SharedPtr<AcDbLine>(line, AcDb::kForWrite));			
//		}
//
//		set_t::iterator iter;
//		// find the oblique one.		
//		sp_t spObligue;
//		for (iter = setLines.begin(); iter != setLines.end(); ++iter)
//		{
//			spObligue = (*iter);
//			if (1.0 < std::abs(spObligue->startPoint().y - spObligue->endPoint().y))
//			{
//				setLines.erase(iter);
//				break;
//			}						
//		}
//
//		if (setLines.end() == iter)
//		{
//			return;
//		}			
//
//		// fine the drop line.
//		iter = min_element(setLines.begin(), setLines.end(), _lowerY);
//		sp_t spDropped = *iter;
//		setLines.erase(iter);
//		xssert(spDropped);				 
//
//		// only 2 bar line left, find the smaller side & bigger side line.
//		double midXDropped = (spDropped->startPoint().x + spDropped->endPoint().x) / 2;
//		double midXOblique = (spObligue->startPoint().x + spObligue->endPoint().x) / 2;
//		sp_t spBigger = *setLines.begin(), spSmaller;
//		double midX = (spBigger->startPoint().x + spBigger->endPoint().x) / 2;
//		
//		xssert(2 == setLines.size());
//		// spBigger is in the same side as spDropped.
//		if (((midX - midXOblique) * (midXDropped - midXOblique)) > 0)
//		{
//			spSmaller = *(++setLines.begin());
//		}
//		else
//		{
//			spSmaller = spBigger;
//			spBigger = *(++setLines.begin());
//		}
//		spSmaller->highlight();
//
//		// adjust the four lines.
//		AcGePoint3d p3dDroppedStart(spDropped->startPoint());
//		AcGePoint3d p3dDroppedEnd(spDropped->endPoint());
//		p3dDroppedStart.x = spDim->xLine1Point().x;
//		p3dDroppedEnd.x = spDim->xLine2Point().x;
//
//		spDropped->setStartPoint(p3dDroppedStart);
//		spDropped->setEndPoint(p3dDroppedEnd);		
//		
//		midXDropped = (spDropped->startPoint().x + spDropped->endPoint().x) / 2;		
//		double xFixedEnd = bXLine1PointFixed ? (spDim->xLine1Point().x) : 
//			(spDim->xLine2Point().x);
//
//		// move the oblique line.
//		AcGeMatrix3d mat;
//		double xObliqueLowerPt = (spObligue->startPoint().y < spObligue->endPoint().y) ? 
//			(spObligue->startPoint().x) : (spObligue->endPoint().x);
//		mat.setTranslation(AcGeVector3d(xFixedEnd - xObliqueLowerPt, 0.0, 0.0));
//		spObligue->transformBy(mat);
//		
//		// adjust the 2 top bars.
//		if (spSmaller->startPoint().x < spBigger->startPoint().x)
//		{
//			// smaller bar is on the left.	
//			
//			// set bigger bar's left end.
//			AcGePoint3d p3d = CGwaLine(spBigger).ucsLeftPoint();
//			p3d.x = CGwaLine(spDropped).ucsLeftPoint().x;
//			CGwaLine(spBigger).setUcsLeftPoint(p3d);
//
//			// set smaller bar's right end.
//			p3d = CGwaLine(spSmaller).ucsRightPoint();
//			p3d.x = CGwaLine(spObligue).ucsLeftPoint().x;
//			CGwaLine(spSmaller).setUcsRightPoint(p3d);
//		}
//		else
//		{
//			// smaller bar is on the right.
//			
//			// set bigger bar's right end
//			AcGePoint3d p3d = CGwaLine(spBigger).ucsRightPoint();
//			p3d.x = CGwaLine(spDropped).ucsRightPoint().x;
//			CGwaLine(spBigger).setUcsRightPoint(p3d);
//
//			// set smaller bar's left end.
//			p3d = CGwaLine(spSmaller).ucsLeftPoint();
//			p3d.x = CGwaLine(spObligue).ucsRightPoint().x;
//			CGwaLine(spSmaller).setUcsLeftPoint(p3d);
//		}
//	}	
//}