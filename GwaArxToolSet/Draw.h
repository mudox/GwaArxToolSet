#pragma once

#include "Util.h"

namespace GwaArx
{
	namespace Draw
	{
		AcDbObjectId aPoint( const AcGePoint3d & pt );
		AcDbObjectId aRectangle( const ads_point corner1, const ads_point corner2 );
		AcDbObjectId aPolyBox( 
			AcGePoint3dArray	ptArr , 
			Adesk::Boolean		closed = Adesk::kTrue,
			bool				bHighlight = true,
			Adesk::UInt16		color = 7,
			std::wstring		layer = TEXT("0"));			
	}
}