#pragma once

#include "StdAfx.h"

#include "Util.h"
#include "Text.h"
#include "Beam.h"

void JustaTest( void )
{
	ads_name n1, n2;
	ads_point pt1, pt2;

	ret_RTNORM(::acedEntSel(NULL, n1, pt1));
	ret_RTNORM(::acedEntSel(NULL, n2, pt2));

	::acedCommand(RTSTR, TEXT("_.OFFSET"),
		RTSHORT, 40,
		RTENAME, n2,
		RTPOINT, pt1,
		RTSTR, TEXT(""),
		RTNONE);

	ads_name n3;
	ret_RTNORM(::acdbEntLast(n3));

	::acedCommand(RTSTR, TEXT("_.FILLET"),
		RTLB, RTENAME, n1, RTPOINT, pt1, RTLE,
		RTLB, RTENAME, n3, RTPOINT, pt2, RTLE,
		RTNONE);	
}

