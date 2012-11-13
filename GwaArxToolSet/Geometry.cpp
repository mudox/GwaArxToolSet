#include "StdAfx.h"

#include "Geometry.h"


bool GwaArx::Geometry::isVertical( dbline_sp line )
{
	return (line->endPoint() - line->startPoint()).isParallelTo(AcGeVector3d(0, 1, 0));
}

bool GwaArx::Geometry::isHorizontal( dbline_sp line )
{
	return (line->endPoint() - line->startPoint()).isParallelTo(AcGeVector3d(1, 0, 0));
}
