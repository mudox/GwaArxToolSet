#pragma once

#include "StdAfx.h"

#include "Util.h"
#include "Text.h"
#include "Beam.h"

void Test1( void )
{
	using namespace GwaArx::Util;
	using namespace GwaArx::TextPatterns;
	/******************* test acedPick & acedPickA ******************
	

	BOOST_AUTO(p, acedPickA<CPatMainBar>());
	BOOST_AUTO(dias, p->barDias());

	acutPrintf(TEXT("\nYour selection: %s, %d Bar Diameter(s):\tmin:\t%d\tmax:\t%d"), 
	p->Text().c_str(), dias.size(), p->minmaxBarDia().first, p->minmaxBarDia().second);
	****************************************************************/
	
	/******************** beam length check ************************
	GwaArx::BeamClac::cmdBeamLenChck();
	****************************************************************/

	GwaArx::Beam::cmdLappingAdjust();
	
	return;
}