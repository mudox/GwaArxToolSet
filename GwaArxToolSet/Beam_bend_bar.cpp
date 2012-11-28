#include "StdAfx.h"

#include "Beam_bend_bar.h"

#include "Util.h"
#include "Geometry.h"
#include "Draw.h"

namespace
{
	typedef std::vector<dbline_sp>				bars_queue;
	typedef std::vector<ent_name>				ename_vec;

	inline bool _isOnLayer(dbline_sp line, std::wstring layerName)
	{
		return layerName == std::wstring(line->layer());
	}

	bool _isBar( dbline_sp theLine )
	{
		using namespace GwaArx::Util;
		heap_str_sp layerName = heap_str2sp(theLine->layer());

		if(std::wstring(TEXT("MAINBAR")) == std::wstring(layerName.get()))
		{			
			return true;			
		}
		else
		{			
			return dbline_sp();
		}		
	}

	inline bool _isRefLine(dbline_sp line)
	{
		// should be on layer "LAYOUT"
		if (!_isOnLayer(line, TEXT("LAYOUT")))
		{
			return false;
		}
		
		// should be vertical in WCS
		if (!GwaArx::Geometry::isVertical(line))
		{
			return false;
		}

		return true;
	}

	dbline_sp _sentryBar( const AcDbObjectId & id )
	{
		// must be a AcDbLine object.
		dbline_sp theLine = GwaArx::Util::id2sp<AcDbLine>(id);
		if (!theLine)
		{
			return dbline_sp();
		}

		return _isBar(theLine) ? theLine : dbline_sp(); 
	}

	dbline_sp _sentryRef( const AcDbObjectId & id )
	{
		// must be a AcDbLine object.
		return GwaArx::Util::id2sp<AcDbLine>(id);		
	}

	unsigned _determineOffsetDistantce(const ads_name & refLineName)
	{
		dbline_sp refLine = GwaArx::Util::name2sp<AcDbLine>(refLineName);
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

	inline bool _fromTop2Bottom(dbline_sp lhs, dbline_sp rhs)
	{
		xssert(lhs && rhs && (lhs.get() != rhs.get()));

		return lhs->startPoint().y > rhs->startPoint().y;
	}

	bool _splitBars(bars_queue & allBars, bars_queue & topBars, 
		bars_queue & bottomBars)
	{
		// at least 1 top bar and 1 bottom bar in it.
		xssert(allBars.size() >= 2);

		// sort first, from top to bottom.
		std::sort(allBars.begin(), allBars.end(), _fromTop2Bottom);

		// the first bar must be the top-most bar.
		topBars.push_back(allBars[0]);

		bool foundTopBottomBoundary = false;
		for (int n = 1; n != allBars.size(); ++n)
		{
			double distantceToUpperBar = (allBars[n - 1]->startPoint().y 
				- allBars[n]->startPoint().y);
			xssert(distantceToUpperBar); // not overlapped.

			const double barSpace = 60.0; // maybe configurable.

			if (std::abs(distantceToUpperBar - barSpace) < 1.0) // tolerance 1.0
			{
				if (!foundTopBottomBoundary)
				{
					topBars.push_back(allBars[n]);
				}
				else
				{
					bottomBars.push_back(allBars[n]);
				}				
			}
			else if (distantceToUpperBar < barSpace)
			{
				xssert(false);
				::acutPrintf(TEXT("\n*发现两根筋之间距离小于 %f *"), barSpace);
				return false;
			}
			else // found a bar from which the distance to its upper bar is greater than 30.0
			{
				if (foundTopBottomBoundary)
				{
					xssert(!"may be selected a sidebar"); 
					::acutPrintf(TEXT("\n*多对相邻钢筋的间距大于 %f *"), barSpace);
					return false;
				}
				else
				{
					foundTopBottomBoundary = true;
					bottomBars.push_back(allBars[n]);
				}
			}
		} // end of for (int n = 1; n != allLines.size(); ++i)

		if (!foundTopBottomBoundary)
		{
			xssert(false);
			::acutPrintf(TEXT("\n*选择集中必须同时有顶筋和底筋*"));
			return false;
		}

		// reverse order of the bottom bars queue,
		// so that the two queue are both listed from outer to inner.
		xssert(bottomBars.size());
		std::reverse(bottomBars.begin(), bottomBars.end());

		xssert(topBars.size() && bottomBars.size());
		return true;
	}

	bool _ssSentry( ads_name ss, bars_queue & topBars, bars_queue & bottomBars, 
		dbline_sp & refLine, ename_vec & entToDel )
	{
		using namespace GwaArx::Util;			
		using namespace boost;
		using GwaArx::Geometry::isVertical;
		using GwaArx::Geometry::isHorizontal;

		bars_queue allLines;
		long ssLen;
		ret_RTNORM(::acedSSLength(ss, &ssLen));

		// should select at least 3 entities 
		// (i.e. a reference line, a top bar and a bottom bar).
		if (ssLen < 3)
		{
			xssert(false);
			::acutPrintf(TEXT("\n*请至少选择一根参考线，一根顶筋，一根底筋*"));
			return false;
		}	

		// ****************** Step 1 *********************
		//  for every ads_name in selection set, if it is the entity name of a 
		//+ AcDbLine, then open and convert it to a shared_ptr (i.e. dbline_sp),
		//+ else it must represents a "ARC" entity, throw them into argument entToDel
		//+ for later deletion by caller.
		ads_name entName;
		dbline_sp line; // used in loop below.
		for (long n = 0; n != ssLen; ++n)
		{
			ret_RTNORM(::acedSSName(ss, n, entName));

			if (line = name2sp<AcDbLine>(entName))
			{
				// open & share_ptr manager them.
				allLines.push_back(line);
			}
			else
			{
				// collect "ARC"s together for later deletion.
				entToDel.push_back(ent_name(entName));
			}			
		}

		// should select at least 3 lines.
		// (i.e. a reference line, a top bar and a bottom bar).
		if (allLines.size() < 3)
		{
			xssert(false);
			::acutPrintf(TEXT("\n*请至少选择一根参考线，一根顶筋，一根底筋*"));
			return false;
		}	

		// ****************** Step 2 *********************
		// group element by layer		
		BOOST_AUTO(layoutLinesBegIter, std::partition(allLines.begin(), allLines.end(), 
			bind(_isOnLayer, _1, TEXT("MAINBAR"))));
		if (allLines.end() == layoutLinesBegIter)
		{
			xssert(false);
			::acutPrintf(TEXT("\n*需要选中一条（且只能是一条）竖直的，在图层\"LAYOUT\""
				TEXT("上的线作为参考线*")));
			return false;
		}
		else if (allLines.begin() == layoutLinesBegIter)
		{
			xssert(false);
			::acutPrintf(TEXT("\n*需要选中在图层\"MAINBAR\"上的水平的钢筋线*"));
			return false;
		}

		bars_queue mainbarLines, layoutLines;
		std::copy(allLines.begin(), layoutLinesBegIter, 
			std::back_inserter(mainbarLines));
		std::copy(layoutLinesBegIter, allLines.end(),
			std::back_inserter(layoutLines));

		// ****************** Step 3 *********************
		//	separate non-horizontal "MAINBAR" lines and throw them into argument 
		//+ 'entToDel' for later deletion by callers. The remaining lines (at least
		//+ 1 top bar and 1 bottom bar) are bars to be bend.
		BOOST_AUTO(
			nonHorzLinesBegIter, 
			std::partition(
			mainbarLines.begin(), 
			mainbarLines.end(), 
			GwaArx::Geometry::isHorizontal
			)
			);

		// check partition result.
		if (std::distance(mainbarLines.begin(), nonHorzLinesBegIter) < 2)
		{
			xssert(!"see prompt below");
			::acutPrintf(TEXT("\n*需要选择至少一条顶筋和一条底筋*"));
			return false;
		}

		// copy all bars out to a new container.
		bars_queue allBars;
		std::copy(mainbarLines.begin(), nonHorzLinesBegIter, back_inserter(allBars));

		{
			AcDbObjectId id;
			ads_name name; 
			for (BOOST_AUTO(iter, nonHorzLinesBegIter); 
				iter != mainbarLines.end(); 
				++iter)
			{
				id = (*iter)->objectId();
				xssert(id != AcDbObjectId::kNull);

				ret_eOk(::acdbGetAdsName(name, id));
				entToDel.push_back(ent_name(name));
			}
		}

		// ****************** Step 4 *********************
		// pick out the reference line among the "LAYOUT" lines, ignoring the other 
		// (do not erase them).
		BOOST_AUTO(
			refLinesEndIter, 
			std::partition(
				layoutLines.begin(), 
				layoutLines.end(), 
				isVertical
				)
			);
		BOOST_AUTO(numRefLines, std::distance(layoutLines.begin(), refLinesEndIter));

		// configurable prompt string.
		const ACHAR refLineRequest[] = TEXT("在图层\"LAYOUT\"上的，竖直的直线");

		if (0 == numRefLines)
		{
			xssert(!"see prompt below");
			::acutPrintf(TEXT("\n*选择集中未发现合适的参考线（%s）*"), 
				refLineRequest);
			return false;
		}
		else if (numRefLines > 1)
		{
			xssert(!"see prompt below");
			::acutPrintf(TEXT("\n*选择集发现多个可能的参考线（%s），只需要一条参")
				TEXT("考线*"), refLineRequest);
			return false;
		}	
		else
		{						
			refLine = *layoutLines.begin();
		}

		xssert(refLine);

		// ****************** Step 5 *********************
		//  splits the remaining bars into two groups (i.e. top bars and bottom bar
		//+ respectively).		
		
		xssert(refLine);;
		return _splitBars(allBars, topBars, bottomBars);
	}

	inline void _determineLenthenEnd( 
		const ads_name		newAchorLineName, 
		const ads_point		barPickPt, 
		ads_point &			lengthenPoint )
	{
		using namespace GwaArx::Util;

		BOOST_AUTO(anchorSeg, name2sp<AcDbLine>(newAchorLineName));  
		xssert(anchorSeg);
		
		AcGePoint3d end1 = anchorSeg->startPoint();
		AcGePoint3d end2 = anchorSeg->endPoint();	
		double len1 = end1.distanceTo(*((AcGePoint3d *)barPickPt));
		double len2 = end2.distanceTo(*((AcGePoint3d *)barPickPt));
		xssert(len1 > 0 && len2 > 0);

		if ( len1 > len2 )
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

	//  for parameter anchorLength
	//+		anchorLength == 0, let the user lengthen anchor line interactively.
	//+		anchorLength > 0, "LENGHEN" the anchor line to the value of anchorLength.
	//+		anchorLength < 0, give the anchor line a fixed length (i.e. 500).

	void _doBendaBar( 
		const ads_name		& 	barName,
		const ads_point			barPickPt, 
		const ads_name		& 	refLineName,		 
		const ads_point			refLinePickPt,
		ent_name::data_t	*	newAnchorSegNameOut = NULL,
		unsigned long			anchorLength = 0 // defaults to let user "LENGTHEN" interactively.
		)
	{
		using namespace GwaArx::Util;
		
		// verify arguments.	
		xssert(anchorLength >= 0 || anchorLength == -1);
		xssert((barPickPt) && (refLinePickPt) && (&barName) && (&refLineName));

		// ****************** Step 1 *********************
		// "OFFSET" the reference line.
		
		// figure out the offset distance.
		unsigned offsetDistance = _determineOffsetDistantce(refLineName);

		ret_RTNORM(::acedCommand(
			RTSTR, TEXT("_.OFFSET"), 
			RTLONG, offsetDistance, 
			RTENAME, refLineName,
			RTPOINT, barPickPt, 
			RTSTR, TEXT(""), 
			RTNONE
			));			

		ads_name newAnchorSegName;
		ret_RTNORM(::acdbEntLast(newAnchorSegName));

		// ****************** Step 2 *********************		
		// "MATCHPROP" & "FILLET" the bar and the new line.	
		
		ret_RTNORM(::acedCommand(
			RTSTR, TEXT("_.MATCHPROP"), 
			RTENAME, barName,
			RTENAME, newAnchorSegName,
			RTSTR, TEXT(""),
			RTNONE
			));		

		ret_RTNORM(::acedCommand(
			RTSTR, TEXT("_.FILLET"), 
			RTLB, RTENAME, barName,				RTPOINT, barPickPt,		RTLE,
			RTLB, RTENAME, newAnchorSegName,	RTPOINT, refLinePickPt,	RTLE,
			RTNONE
			));		

		// ****************** Step 3 *********************
		// "LENGTHEN" the vertical anchor line.

		// "LENGTHEN" the anchor line to a default length first.
		ads_point lengthenPoint;
		_determineLenthenEnd(newAnchorSegName, barPickPt, lengthenPoint);

		const unsigned short defaultAnchorLength = 500; // this can be configurable.
		ret_RTNORM(::acedCommand(
			RTSTR, TEXT("_.LENGTHEN"),
			RTSTR, TEXT("_T"),
			RTSHORT, ((anchorLength > 0) ? anchorLength : defaultAnchorLength),	
			RTLB, RTENAME, newAnchorSegName, RTPOINT, lengthenPoint, RTLE,
			RTSTR, TEXT(""),
			RTNONE
			));

		// let user "LENGTHEN" anchor line interactively, if required.
 		if (0 == anchorLength)
 		{
  			FreeCursorAWhile _close_os_snap_mode_a_while(
  				static_cast<sysvar_bit>(setORTHOMODE | setSNAPMODE)); 			

 			_determineLenthenEnd(newAnchorSegName, barPickPt, lengthenPoint);	
 
 			ret_RTNORM(::acedCommand(
 				RTSTR, TEXT("_.LENGTHEN"),
 				RTSTR, TEXT("_DY"),		
 				RTLB, RTENAME, newAnchorSegName, RTPOINT, lengthenPoint, RTLE,
 				RTSTR, PAUSE,
 				RTSTR, TEXT(""),
 				RTNONE
 				));
 		}

		// pass out the anchored segment's ads_name, if required.
		if (newAnchorSegNameOut)
		{
			::memcpy(newAnchorSegNameOut, newAnchorSegName, sizeof(ads_name));
		}	
	}

	void _getInput( bars_queue & topBars, bars_queue & bottomBars, 
		dbline_sp & refLine, ename_vec & entToDel)
	{
		using namespace boost;

		resbuf *ssFilter = ::acutBuildList(
			__OR, // ARCs or LINEs
				__AND,
					RTDXF0, TEXT("ARC"), 8, TEXT("MAINBAR"),
				DNA__,
				__AND,
					RTDXF0, TEXT("LINE"),
					__OR,
						8,	TEXT("MAINBAR"), 8,	TEXT("LAYOUT"),
					RO__,	
				DNA__,
			RO__,	
			RTNONE);
		xssert(ssFilter);		

  		GwaArx::Util::CSSGet ssget;
  		ssget
  			.setPrompt(TEXT("\n需要选取至少一条水平顶筋，至少一条水平底筋，以及一条")
 				TEXT("（且只能是一条竖直的梁轮廓参考线）："))
  			.setFilter(ssFilter)
  			.setSentry(bind(_ssSentry, _1, ref(topBars), ref(bottomBars), ref(refLine), ref(entToDel)));
  		ssget();	

		xssert(topBars.size() != 0);
		xssert(bottomBars.size() != 0);
		xssert(refLine);
	}

	double _clacAnchorLength(
		const double maxBarSpacing,
		const double numTopBars, 
		const double numBottomBars, 
		const double level
		)
	{
		xssert(numTopBars && numBottomBars);

		// configurable constants.
		const double endOffset = 40; 
		const double barSpacing = 30; 
		const double filletDia = 60;
		const double minLen = 115;

		double result = maxBarSpacing;

		bool inPairs = false;	

		for(double n = 0; n != level + 1; ++n)
		{
			inPairs = n < (std::min)(numTopBars, numBottomBars);
			result -= (inPairs ? 2 : 1) * filletDia + endOffset;
			
			if (result < minLen)
			{
				return minLen;
			}
		}
		
		return result;
	}
}

void GwaArx::Beam::_bend_bar::cmdBendABar( void )
{
	ads_point	barPickPt,	refLinePickPt;
	ads_name	barName,	refLineName;
	while(1)
	{
		{
			// let the user pick the bar to bend.		
			dbline_sp theBar = GwaArx::Util::acedPick(_sentryBar, 
				TEXT("\n点取钢筋线："), 
				barPickPt);		
			theBar->highlight();

			// let the user pick the referencing line.	
			dbline_sp theRefLine = GwaArx::Util::acedPick(_sentryRef, 
				TEXT("\n点取参考线："), 
				refLinePickPt);	
			theBar->unhighlight();

			// offset reference line.			
			ret_eOk(acdbGetAdsName(refLineName, theRefLine->objectId()));
			ret_eOk(acdbGetAdsName(barName, theBar->objectId()));
		} 
		//  theBar & theRefline release it's occupation of the underlying 
		//+ database objects so the _doBendaBar can successively modify
		//+ the same objects.

	// it will do the dirty work.
	_doBendaBar(barName, barPickPt, refLineName, refLinePickPt);
	}
}

void GwaArx::Beam::_bend_bar::cmdBendBarsAuto( void )
{
	using namespace GwaArx::Util;
	// ****************** Step 1 *********************
	//  get input
	//+ while in _getInput(), selection set is searched for significant object and
	//+ the other is collected into a group, which then is erased after _getInput()
	//+ return.
	
	bars_queue topBars, bottomBars;
	dbline_sp refLine;
	ename_vec entToDel;
	entToDel.reserve(32); // optimizing reservation.

	_getInput(topBars, bottomBars, refLine, entToDel);		

	// erase rubbish.
	std::for_each(entToDel.begin(), entToDel.end(), ::acdbEntDel);

#ifdef MDX_DEBUG
	acutPrintf(TEXT("\n%d top bars, %d bottom bars"), topBars.size(), 
	bottomBars.size());	
#endif	
	
	// ****************** Step 2 *********************
	// bend bars
	
	// get ads_name of the reference line.
	ads_name refLineName;	
	ret_eOk(::acdbGetAdsName(refLineName, refLine->objectId()));
	
	// calculate the pick point on the reference line. 
	double pickPtY = 
		(topBars.back()->endPoint().y + bottomBars.back()->endPoint().y) / 2;
	double pickPtX = 
		refLine->endPoint().x;
	ads_point pickPtOnRefLine = {pickPtX, pickPtY, 0.0};

	MDX_ON_DBG(GwaArx::Draw::aPoint(*(AcGePoint3d *)pickPtOnRefLine));	
	
	// wrap auto variables predefined before loop.
  	{
  		// auto variables used in loop.  		
  		ads_name name;
  		AcGeLineSeg3d barLineSeg;
  		AcGePoint3d pickPtOnBar;
  		ads_name newAchorLineName;
  		AcGePoint3d ignore;
  		long anchorLength = -1;
		double maxBarSpacing = topBars[0]->startPoint().y - 
			bottomBars[0]->startPoint().y; 
 
  		int N = (std::max)(topBars.size(), bottomBars.size());
  		for (int n = 0; n != N; ++n)
  		{
  			// bend a top bar first (if not exhausted)
  			if (n < topBars.size())
  			{
  				// get bar's ads_name  				 				
  				ret_eOk(::acdbGetAdsName(name, topBars[n]->objectId()));
  
  				//  determine pick point on topBars[n].
  				//  one of the two end points whichever is closest to the reference
  				//+ line is chosen to be the pick point ( with assumption that all
  				//+ bars are in the same side of reference line, they do not cross
  				//+ the reference line).
  				barLineSeg.set(topBars[n]->startPoint(), topBars[n]->endPoint());  				
				pickPtOnBar = barLineSeg.midPoint();

 				MDX_ON_DBG(GwaArx::Draw::aPoint(pickPtOnBar));  

  				// calculates anchor length.
  				anchorLength = _clacAnchorLength(maxBarSpacing, topBars.size(), 
					bottomBars.size(), n); 
  	
  				if (0 == n)
  				{
					// release shared_ptr occupation.
// 					xssert(1 == topBars[n].use_count());
// 					topBars[n].reset();					
// 					xssert(1 == refLine.use_count());
// 					refLine.reset();
 					dbSpClose(topBars[n]);
					dbSpClose(refLine);

  					_doBendaBar(
						name, (ads_real *)&pickPtOnBar, 
  						refLineName, pickPtOnRefLine, 
  						newAchorLineName, anchorLength
						);
  				}
  				else
  				{
					// release shared_ptr occupation.
					xssert(1 == topBars[n].use_count());
					topBars[n].reset();				

  					_doBendaBar(
						name, (ads_real *)&pickPtOnBar, 
  						newAchorLineName, pickPtOnRefLine, 
  						newAchorLineName, anchorLength
						);
  				}				
  			}			
 
 			// then bend a bottom bar (if not exhausted).
  			if (n < bottomBars.size())
  			{
  				// get bottomBars[n]'s ads_name.
  				ret_eOk(::acdbGetAdsName(name, bottomBars[n]->objectId()));
  
  				// see above if{..}
  				barLineSeg.set(bottomBars[n]->startPoint(), bottomBars[n]->endPoint());  				
				pickPtOnBar = barLineSeg.midPoint();

  				MDX_ON_DBG(GwaArx::Draw::aPoint(pickPtOnBar)); 

  				//calculate anchor length.
				anchorLength = _clacAnchorLength(maxBarSpacing, topBars.size(), 
					bottomBars.size(), n); 
  
				// release shared_ptr occupation.	
				xssert(1 == bottomBars[n].use_count());
				bottomBars[n].reset();				

  				_doBendaBar(name, (ads_real *)&pickPtOnBar, 
  					newAchorLineName, pickPtOnRefLine, 
  					newAchorLineName, anchorLength);
  			}		
 		} // end of loop to bend bars.
 	} // wrapper of loop.
} // cmdBendBarsAuto().