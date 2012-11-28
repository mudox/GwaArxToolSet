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

AcGeMatrix3d GwaArx::Geometry::matBy2Pt( AcGePoint3d org, AcGePoint3d xPt )
{	
	AcGeVector3d xAxis = (xPt - org).normal();	
	AcGeVector3d zAxis(0, 0, 1);
	AcGeVector3d yAxis = zAxis.crossProduct(xAxis);
	
	AcGeMatrix3d mat;
	mat.setCoordSystem(org, xAxis, yAxis, zAxis);

	return mat;
}
