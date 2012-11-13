#pragma once

#include "StdAfx.h"

#include "Util.h"
#include "Text.h"
#include "Beam.h"
#include "Draw.h"

void JustaTest( void )
{
	static bool t = false;
	
	AcDbAppSystemVariables a;
	a.setOsmode(t = !t ? 0 : 1983);
}

