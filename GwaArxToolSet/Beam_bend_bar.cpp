#include "StdAfx.h"

#include "Util.h"

#include "Beam_bend_bar.h"

namespace
{
	typedef boost::shared_ptr<AcDbLine> 	line_sp;
	typedef line_sp							bar_sp;

	bool _isBar( line_sp theLine )
	{
		using namespace GwaArx::Util;
		achar_sp layerName = achar2sp(theLine->layer());

		if(std::wstring(TEXT("MAINBAR")) == std::wstring(layerName.get()))
		{			
			return true;			
		}
		else
		{			
			return line_sp();
		}		
	}

	bar_sp _sentryBar( const AcDbObjectId & id )
	{
		// must be a AcDbLine object.
		line_sp theLine = GwaArx::Util::id2sp<AcDbLine>(id);
		if (!theLine)
		{
			return line_sp();
		}

		return _isBar(theLine) ? theLine : bar_sp(); 
	}

	line_sp _sentryRef( const AcDbObjectId & id )
	{
		// must be a AcDbLine object.
		return GwaArx::Util::id2sp<AcDbLine>(id);		
	}

	unsigned _determineOffsetDistantce(const ads_name & refLineName)
	{
		line_sp refLine = GwaArx::Util::name2sp<AcDbLine>(refLineName);
		xssert(refLine);
		
		if (_isBar(refLine))
		{
			AcGeVector3d refLineVector = refLine->startPoint() - refLine->endPoint();
			AcGeVector3d yAxis(0, 1, 0);
			if (refLineVector.isParallelTo(yAxis))
			{
				return 30;
			}
			else
			{
				return 60;
			}			
		}
		else
		{
			return 40;
		}
	}


	inline void _determineLenthenEnd( 
		const ads_name	& anchorSegName, 
		const ads_point	& barPickPt, 
		ads_point &	lengthenPoint )
	{
		using namespace GwaArx::Util;

		BOOST_AUTO(anchorSeg, name2sp<AcDbLine>(anchorSegName));
		BOOST_AUTO(end1, anchorSeg->startPoint());
		BOOST_AUTO(end2, anchorSeg->endPoint());	
		if (
			end1.distanceTo(*((AcGePoint3d *)&barPickPt)) > 
			end2.distanceTo(*((AcGePoint3d *)&barPickPt))
			)
		{
			lengthenPoint[0] = end1.x;
			lengthenPoint[1] = end1.y;
			lengthenPoint[2] = end1.z;
		}
		else
		{
			lengthenPoint[0] = end2.x;
			lengthenPoint[1] = end2.y;
			lengthenPoint[2] = end2.z;
		}		
	}

	void _doBendaBar( 
		const ads_name	& 	barName,
		const ads_point	& 	barPickPt, 
		const ads_name	& 	refLineName,		 
		const ads_point	& 	refLinePickPt )
	{
		using namespace GwaArx::Util;

		// figure out the offset distance.
		unsigned offsetDistance = _determineOffsetDistantce(refLineName);

		::acedCommand(RTSTR, TEXT("_.OFFSET"), 
			RTLONG, offsetDistance, 
			RTENAME, refLineName,
			RTPOINT, barPickPt, 
			RTSTR, TEXT(""), 
			RTNONE);	

		// fillet the bar and the new line.	
		ads_name anchorSegName;
		ret_RTNORM(::acdbEntLast(anchorSegName));

		::acedCommand(RTSTR, TEXT("_.MATCHPROP"), 
			RTENAME, barName,
			RTENAME, anchorSegName,
			RTSTR, TEXT(""),
			RTNONE);

		::acedCommand(RTSTR, TEXT("_.FILLET"), 
			RTLB, RTENAME, barName,				RTPOINT, barPickPt,		RTLE,
			RTLB, RTENAME, anchorSegName,	RTPOINT, refLinePickPt,	RTLE,
			RTNONE);	

		ads_point lengthenPoint;
		_determineLenthenEnd(anchorSegName, barPickPt, lengthenPoint);

		FreeCursorAWhile _close_os_snap_mode_a_while(
			static_cast<sysvar_bit>(setORTHOMODE | setSNAPMODE));

		::acedCommand(RTSTR, TEXT("_.LENGTHEN"),
			RTSTR, TEXT("_T"),
			RTSHORT, 500,	// default length 
			RTLB, RTENAME, anchorSegName, RTPOINT, lengthenPoint, RTLE,
			RTSTR, TEXT(""),
			RTNONE);

		// recalculation is necessary because the line length is changed.
		_determineLenthenEnd(anchorSegName, barPickPt, lengthenPoint);


		::acedCommand(RTSTR, TEXT("_.LENGTHEN"),
			RTSTR, TEXT("_DY"),		
			RTLB, RTENAME, anchorSegName, RTPOINT, lengthenPoint, RTLE,
			RTSTR, PAUSE,
			RTSTR, TEXT(""),
			RTNONE);
	}
}

void GwaArx::Beam::_bend_bar::cmdBendBar( void )
{
	ads_point	barPickPt,	refLinePickPt;
	ads_name	barName,	refLineName;

	while(1)
	{
		{
			// let the user pick the bar to bend.		
			bar_sp theBar = GwaArx::Util::acedPick(_sentryBar, 
				TEXT("\n点取钢筋线："), 
				barPickPt);		
			theBar->highlight();

			// let the user pick the referencing line.	
			line_sp theRefLine = GwaArx::Util::acedPick(_sentryRef, 
				TEXT("\n点取参考线："), 
				refLinePickPt);	
			theBar->unhighlight();

			// offset reference line.			
			ret_eOk(acdbGetAdsName(refLineName, theRefLine->objectId()));
			ret_eOk(acdbGetAdsName(barName, theBar->objectId()));
		} 
		//  theBar & theRefline release it's occupation of underlying 
		//+ database objects so the _doBendaBar can successively modify
		//+ the same objects.

	// it will do the dirty work.
	_doBendaBar(barName, barPickPt, refLineName, refLinePickPt);
	}
}

void GwaArx::Beam::_bend_bar::cmdBendBarAuto( void )
{
	
}