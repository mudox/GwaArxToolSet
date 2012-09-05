#pragma once

#include "Debug.h"
#include "Util_acdb_shared_ptr.h" 
#include "Util_acedget.h"
#include "Util_misc.h"
#include "Util_string_cast.h"
#include "Util_arx_cmd_helpers.h"

// global inline routine to get current dwg's AcDbDatabase *
inline AcDbDatabase * curDwg( void )
{
	return acdbHostApplicationServices()->workingDatabase();
}

namespace GwaArx
{
	namespace Util
	{			
		using namespace GwaArx::Util::_arx_shared_ptr;
		using namespace GwaArx::Util::_aced_get;
		using namespace GwaArx::Util::_misc;
		using namespace GwaArx::Util::_string_cast;
		using namespace GwaArx::Util::_arx_cmd_helpers;
	}
}

