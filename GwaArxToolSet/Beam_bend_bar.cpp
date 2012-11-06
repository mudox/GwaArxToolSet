#include "StdAfx.h"

#include "Util.h"
#include "Geometry.h"

#include "Beam_bend_bar.h"


namespace
{
	typedef boost::shared_ptr<AcDbLine> 	line_sp;
	typedef line_sp							bar_sp;
	typedef boost::shared_ptr<AcDbArc>		arc_sp;
	typedef std::vector<bar_sp>				bars_queue;

	inline bool _isOnLayer(line_sp line, std::wstring layerName)
	{
		return layerName == std::wstring(line->layer());
	}

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

	inline bool _isRefLine(line_sp line)
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

	inline bool _fromTop2Bottom(bar_sp lhs, bar_sp rhs)
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
					xssert(false);
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
	bool _ssSentry_old(ads_name ss, bars_queue & topBars, bars_queue & bottomBars, 
		line_sp & refLine)
	{
		using namespace GwaArx::Util;		

		bars_queue allLines;
		long ssLen;
		ret_RTNORM(::acedSSLength(ss, &ssLen));
		// at least a reference line, a top bar and a bottom bar is selected.
		if (ssLen < 3)
		{
			xssert(false);
			::acutPrintf(TEXT("\n*请至少选择一根参考线，一根顶筋，一根底筋*"));
			return false;
		}	 
		
		// ****************** Step 1 *********************		
		// transform ads_name of selection set to vector<line_sp>
					
		ads_name entName;
		for (long n = 0; n != ssLen; ++n)
		{
			ret_RTNORM(::acedSSName(ss, n, entName));
			line_sp line = name2sp<AcDbLine>(entName);
			xssert(line);

			allLines.push_back(line);
		}

		// ****************** Step 2 *********************		
		// find out the reference line.
		// one and only one reference line should be found in the selection set.		
		bars_queue refLines;
		BOOST_AUTO(refLineIter, std::partition(allLines.begin(), allLines.end(), 
			std::not1(std::ptr_fun(_isRefLine))));
		BOOST_AUTO(numRefLines, std::distance(refLineIter, allLines.end()));		

		if (0 == numRefLines)
		{
			xssert(false);
			::acutPrintf(TEXT("\n*选择集中未发现合适的参考线*"));
			return false;
		}
		else if (numRefLines > 1)
		{
			xssert(false);
			::acutPrintf(TEXT("\n*选择集发现多个参考线，只需要一条参考线*"));
			return false;
		}	
		else
		{			
			refLine = *refLineIter;
		}

		// cut the tail (i.e. the only one reference line).
		allLines.pop_back();
		
		// ****************** Step 3 *********************		
		// grouping the remaining bars into two groups for top bars and bottom bar
		// respectively.		
		
		// assert all remaining bars should be horizontal bars.
		for (int n = 0; n != allLines.size(); ++n)
		{
			if (!GwaArx::Geometry::isHorizontal(allLines[n]))
			{
				xssert(false);
				::acutPrintf(TEXT("\n*所有的钢筋必须是水平的*"));
				return false;
			}
		}

		// sort first, from top to bottom.
		std::sort(allLines.begin(), allLines.end(), _fromTop2Bottom);

		// the first bar must be the top-most bar.
		topBars.push_back(allLines[0]);

		bool foundTopBottomBoundary = false;
		for (int n = 1; n != allLines.size(); ++n)
		{
			double distantceToUpperBar = (allLines[n - 1]->startPoint().y 
				- allLines[n]->startPoint().y);
			xssert(distantceToUpperBar);

			const double barSpace = 60.0; // maybe configurable.

			if (std::abs(distantceToUpperBar - barSpace) < 1.0) // tolerance 1.0
			{
				if (!foundTopBottomBoundary)
				{
					topBars.push_back(allLines[n]);
				}
				else
				{
					bottomBars.push_back(allLines[n]);
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
					xssert(false);
					::acutPrintf(TEXT("\n*多对相邻钢筋的间距大于 %f *"), barSpace);
					return false;
				}
				else
				{
					foundTopBottomBoundary = true;
					bottomBars.push_back(allLines[n]);
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

		xssert(topBars.size() && bottomBars.size() && refLine);
		return true;
	}	

	bool _ssSentry(ads_name ss, bars_queue & topBars, bars_queue & bottomBars, 
		line_sp & refLine)
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
		// transform ads_name of selection set to vector<line_sp> and erase all arcs.		
		ads_name entName;
		line_sp line;
		std::vector<long> arcIndices;
		for (long n = 0; n != ssLen; ++n)
		{
			ret_RTNORM(::acedSSName(ss, n, entName));

			if (line = name2sp<AcDbLine>(entName))
			{
				allLines.push_back(line);
			}
			else
			{
				arcIndices.push_back(n);
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
		
		// erase arcs.
		ads_name arcName;
		for (long n = 0; n != arcIndices.size(); ++n)
		{
			ret_RTNORM(::acedSSName(ss, arcIndices[n], arcName));
			ret_RTNORM(::acdbEntDel(arcName));
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
		// pick out the reference line among the "LAYOUT" lines group, ignoring the
		// others (do not erase them).
		BOOST_AUTO(
			refLinesEndIter, 
			std::partition(
				layoutLines.begin(), 
				layoutLines.end(), 
				isVertical
				)
			);
		BOOST_AUTO(numRefLines, std::distance(layoutLines.begin(), refLinesEndIter));

		if (0 == numRefLines)
		{
			xssert(false);
			::acutPrintf(TEXT("\n*选择集中未发现合适的参考线*"));
			return false;
		}
		else if (numRefLines > 1)
		{
			xssert(false);
			::acutPrintf(TEXT("\n*选择集发现多个参考线，只需要一条参考线*"));
			return false;
		}	
		else
		{			
			refLine = *refLinesEndIter;
		}

		xssert(refLine);

		// ****************** Step 4 *********************
		// separate non-horizontal "MAINBAR" lines and erase them.
		// the remaining lines (at least 1 top bar and 1 bottom bar) are bars to be
		// bend.
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
			xssert(false);
			::acutPrintf(TEXT("\n*需要选择至少一条顶筋和一条底筋*"));
			return false;
		}
		// copy all bars out to a new container.
		bars_queue allBars;
		std::copy(mainbarLines.begin(), nonHorzLinesBegIter, back_inserter(allBars));
		// erase the other.
		for (BOOST_AUTO(iter, nonHorzLinesBegIter); iter != mainbarLines.end(); ++iter)
		{
			ret_eOk((*iter)->upgradeOpen());
			ret_eOk((*iter)->erase());
		}

		// ****************** Step 5 *********************
		// grouping the remaining bars into two groups (i.e. top bars and bottom bar
		// respectively).
		return _splitBars(allBars, topBars, bottomBars);
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
		const ads_point	& 	refLinePickPt,
		short				anchorLength = -1
		)
	{
		using namespace GwaArx::Util;
		
		// verify arguments.
		xssert(anchorLength > 0 || -1 == anchorLength);
		xssert((&barPickPt) && (&refLinePickPt) && (&barName) && (&refLineName));

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

		const unsigned short defaultAnchorLength = 500; // this can be configurable.
		::acedCommand(RTSTR, TEXT("_.LENGTHEN"),
			RTSTR, TEXT("_T"),
			RTSHORT, (anchorLength < 0) ? defaultAnchorLength : anchorLength,	
			RTLB, RTENAME, anchorSegName, RTPOINT, lengthenPoint, RTLE,
			RTSTR, TEXT(""),
			RTNONE);
		
		if (anchorLength < 0)
		{
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

	void _getInput(bars_queue & topBars, bars_queue & bottomBars, line_sp refLine)
	{
		using namespace boost;

		//  select "LINE"s on layers of either "MAINBAR" or "LAYOUT" or "ARC"s on
		//+ layer "MAINBAR"
// 		resbuf *ssFilter = ::acutBuildList(
// 		-4, TEXT("<OR"), // ARCs or LINEs
// 			-4, TEXT("<AND"),
// 				RTDXF0, TEXT("ARC"), 8, TEXT("MAINBAR"),
// 			-4, TEXT("AND>"),
// 			-4, TEXT("<AND"),
// 				RTDXF0, TEXT("LINE"),
// 				-4, TEXT("<OR"),
// 					8,	TEXT("MAINBAR"),
// 					8,	TEXT("LAYOUT"),
// 				-4, TEXT("OR>"),	
// 			-4, TEXT("AND>"),
// 		-4, TEXT("OR>"),	
// 		RTNONE);	
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

  		GwaArx::Util::CAcEdSSGet ssget;
  		ssget
  			.setPrompt(TEXT("\n需要选取至少一条水平顶筋，至少一条水平底筋，以及一条")
 				TEXT("（且只能是一条竖直的梁轮廓参考线）："))
  			.setFilter(ssFilter)
  			.setSentry(bind(_ssSentry, _1, ref(topBars), ref(bottomBars), ref(refLine)));
  		ssget();
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

void GwaArx::Beam::_bend_bar::cmdBendBarsAuto( void )
{
	// ****************** Step 1 *********************
	// get input
	bars_queue topBars, bottomBars;
	line_sp theRefLine;
	_getInput(topBars, bottomBars, theRefLine);	

	acutPrintf(TEXT("\n%d top bars, %d bottom bars"), topBars.size(), bottomBars.size());
	
	// ****************** Step 3 *********************
	// bend bars
}