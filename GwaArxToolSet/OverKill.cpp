#include "StdAfx.h"

#include "Util.h"
#include "Geometry.h"
#include "Draw.h"
#include "OverKill.h"

namespace
{
	template<typename OutputIterator>
	void _lineNearFieldScan(
		AcDbObjectId		lineId, 
		heap_rb_sp			filterList, 
		OutputIterator		outIter,
		bool				exludeSelf = true,
		double				fuzz = 0
		)
	{
		using namespace GwaArx::Util;

		AcGePoint3dArray pntArr;
		pntArr.setLogicalLength(4);

		{
			dbline_sp theLine = id2sp<AcDbLine>(lineId);
			xssert(theLine);
	
			// Calculate the capture region.

			pntArr[0] = theLine->startPoint();
			pntArr[2] = theLine->endPoint();	
		} // theLine closed

		AcGeMatrix3d mat = GwaArx::Geometry::matBy2Pt(pntArr[0], pntArr[2]);

		mat.invert();
		pntArr[0].transformBy(mat);
		pntArr[2].transformBy(mat);
		
		// Prepare fuzz value.
		fuzz = fuzz ? fuzz : 0.5; // configurable

		pntArr[1] = pntArr[0];
		pntArr[3] = pntArr[2];
		pntArr[0].x -= fuzz;
		pntArr[0].y -= fuzz;
		pntArr[1].x -= fuzz;
		pntArr[1].y += fuzz;
		pntArr[2].x += fuzz;
		pntArr[2].y += fuzz;
		pntArr[3].x += fuzz;
		pntArr[3].y -= fuzz;

		mat.invert();
		for (int n = 0; n != 4; ++n)
		{
			pntArr[n].transformBy(mat);
		}

#ifdef MDX_DEBUG
		GwaArx::Draw::aPolyBox(pntArr);	
#endif 

		ads_name ss;
		CSSGet ssget;

		heap_rb_sp cpPntList = heap_rb2sp(::acutBuildList(
			RTPOINT, asDblArray(pntArr[0]),
			RTPOINT, asDblArray(pntArr[1]),
			RTPOINT, asDblArray(pntArr[2]),
			RTPOINT, asDblArray(pntArr[3]),
			RTNONE));
		xssert(cpPntList);

		ssget
			.setStr(TEXT("CP")) // crossing polygon selection
			.setSS(ss)
			.setFilter(filterList.get())
			.setPt1(cpPntList.get());
		ssget();	

		std::vector<AcDbObjectId> ids;
		ss2Ids(ss, std::back_inserter(ids));
		// exclude self if required.
		if (exludeSelf)
		{			
			std::vector<AcDbObjectId>::iterator selfPos = 
				std::find(ids.begin(), ids.end(), lineId);
			xssert(selfPos != ids.end());
			ids.erase(selfPos);
		}

// #ifdef MDX_DEBUG
// 		// Highlight selected entities		
// 		idsHighlight(ids.begin(), ids.end());
// #endif				
		std::copy(ids.begin(), ids.end(), outIter);
		ret_RTNORM(::acedSSFree(ss));
	} // void _lineNearFieldScan(...)


	bool _isOverLapping( 
		const AcDbObjectId & lhs, 
		const AcDbObjectId & rhs )
	{		
		using namespace GwaArx::Util;
		dbline_sp l = id2sp<AcDbLine>(lhs);
		dbline_sp r = id2sp<AcDbLine>(rhs);
				
		bool onEqualLayer = toWStr(l->layer()) == toWStr(r->layer());
		

		if(
			onEqualLayer &&										// on the same layer
			(l->colorIndex() == r->colorIndex()) &&				// same color
			(l->linetypeId() == r->linetypeId()) &&				// same line type
			(l->linetypeScale() == r->linetypeScale()) &&		// same line type scale
			(l->lineWeight() == r->lineWeight())				// same line plot weight
			)
		{ // then the most important condition: they must be collinear to each other.
			AcGeLineSeg3d lSeg(l->startPoint(), l->endPoint());
			AcGeLineSeg3d rSeg(r->startPoint(), r->endPoint());

			//  we do not use isColinearTo() here is because the distances fuzz is
			//+ controlled by the variable 'fuzz' previously defined which is used
			//+ to calculate the near field scan box, the near field box in turn is
			//+ used to actually control the distance fuzz.
			return lSeg.isParallelTo(rSeg); 
		}	
		else
		{
			return false;
		}
	} // bool _isOverLapping(...)

	bool _pointLessX( const AcGePoint3d & l, const AcGePoint3d & r )
	{
		return l.x < r.x;
	}
} // anonymous namespace

//  The ok*** functions will delete or write an abitrary number of entities during
//+ it's running. So make sure "NO ENTITIES IN DATABASE IS OPENED" before invoking
//+ these functions.
void GwaArx::OverKill::okLine( AcDbObjectId lineId )
{
	using namespace GwaArx::Util;

	heap_str_sp	layer;
	{ // the sample line is opened for reading to get the layer string.
		dbline_sp theLine = GwaArx::Util::id2sp<AcDbLine>(lineId);
		xssert(theLine && theLine.unique());	
		layer = heap_str2sp(theLine->layer());	
	} // the sample line is opened for reading to get the layer string.
	
	// ****************** Step 1 *********************
	// Apply "near field scanning".
	
	// Build filter list.
	heap_rb_sp filterList = heap_rb2sp(::acutBuildList(
		RTDXF0, TEXT("LINE"), 			
 		8, layer.get(),
		RTNONE));
	xssert(filterList);

	typedef std::vector<AcDbObjectId> id_vec;	
	id_vec capturedLineIds;
	_lineNearFieldScan(lineId, filterList, std::back_inserter(capturedLineIds));	

	if (capturedLineIds.empty())
	{
		// no overlapping line found.
		// unwind recursing stacks.
		return; 
	}	
	
	// ****************** Step 2 *********************
	// Filter out lines to delete
	
	id_vec::iterator overLineEnd = 
		std::partition(capturedLineIds.begin(), capturedLineIds.end(),
		BOOST_BIND(_isOverLapping, lineId, _1));

	id_vec overLineIds;
	if (overLineEnd == capturedLineIds.begin())
	{
		// no overlapping line found.
		// unwind recursing stacks.
		return;
	}
	else
	{
		std::copy(capturedLineIds.begin(), overLineEnd, 
			std::back_inserter(overLineIds));
	}

#ifdef MDX_DEBUG
	// Highlight overlapping lines
	::acutPrintf(TEXT("\n%d overlapping line found"), overLineIds.size());
	idsHighlight(overLineIds.begin(), overLineIds.end());
#endif	

	// ****************** Step 3 *********************
	// figures out the the two new ending points.
	AcGePoint3d endPt1, endPt2;
	{ // all overlapping lines and the sample line are opened for reading.
		std::vector<dbline_sp> overLines;
		// converts vector of ids to vector of shared_ptrs.
 		std::transform(
			overLineIds.begin(), overLineIds.end(),
 			std::back_inserter(overLines), 
			BOOST_BIND(id2sp<AcDbLine>, _1, AcDb::kForRead, false)
			);
		
 		//  get the two ending points of each overlapping line, and collect them into 
 		//+ a vector.
 		std::vector<AcGePoint3d> endPoints;	
 		std::transform(overLines.begin(), overLines.end(), 
 			std::back_inserter(endPoints), BOOST_BIND(&AcDbLine::startPoint, _1));
 		std::transform(overLines.begin(), overLines.end(), 
 			std::back_inserter(endPoints), BOOST_BIND(&AcDbLine::endPoint, _1));

		dbline_sp theLine = id2sp<AcDbLine>(lineId);
 		endPoints.push_back(theLine->startPoint());
 		endPoints.push_back(theLine->endPoint());
 
#ifdef MDX_DEBUG
		// Highlight overlapping lines
		::acutPrintf(TEXT("%d points in vector endPoints"), endPoints.size());
#endif	

 		// transform them in to source line's ECS.	 		
 		AcGeMatrix3d mat = GwaArx::Geometry::matBy2Pt(
 			theLine->startPoint(), theLine->endPoint());
 
 		mat.invert();
 		std::for_each(endPoints.begin(), endPoints.end(),
 			BOOST_BIND(&AcGePoint3d::transformBy, _1, mat));	
 
 		BOOST_AUTO(boundingPoints, 
			boost::minmax_element(endPoints.begin(), endPoints.end(), _pointLessX));
		endPt1 = *boundingPoints.first;
		endPt2 = *boundingPoints.second;
		
		// transform the two resulting new ending points back to WCS
		mat.invert();
		endPt1.transformBy(mat);
		endPt2.transformBy(mat);	
	
	// ****************** Step 4 *********************
	// Erase overlapping lines and reset sample line's two ending points.
		for (BOOST_AUTO(iter, overLines.begin());
			iter != overLines.end(); ++iter)
		{
			(*iter)->upgradeOpen();
			(*iter)->erase();
		}

		theLine->upgradeOpen();
		theLine->setStartPoint(endPt1);
		theLine->setEndPoint(endPt2);	
	} // all overlapping lines and the sample line are opened for reading.

	// ****************** Step 5 *********************
	//  Recurse the the whole progress on the new sample line until no overlapping
	//+ line can be found.
	okLine(lineId);
} // void GwaArx::OverKill::okLine( AcDbObjectId lineId ) 

