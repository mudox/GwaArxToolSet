#pragma once

#include "StdAfx.h"

namespace GwaArx
{
	namespace Util
	{
		namespace _misc
		{
			void VerifyBarDia( unsigned dia );			

			ads_real * asAdsPoint( AcGePoint3d & p3d );			

			const ads_real * asAdsPoint( const AcGePoint3d & p3d );			
		}
	}
}