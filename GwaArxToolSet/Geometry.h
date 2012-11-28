#pragma once

#include "Util.h"

namespace GwaArx
{
	namespace Geometry
	{		
		bool isVertical( dbline_sp line );
		bool isHorizontal( dbline_sp line );		
		AcGeMatrix3d matBy2Pt( AcGePoint3d pointAsOrg, AcGePoint3d pointOnPosX );
	}
}