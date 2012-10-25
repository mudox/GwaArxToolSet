#pragma once

#include "StdAfx.h"

#include "Util.h"
#include "Text.h"
#include "Beam.h"

void Foo(void);

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

	GwaArx::Beam::cmdAdjustLapping();
	//Foo();
	return;
}

void Foo( void )
{
	using namespace GwaArx::Util;
	using namespace boost;

	BOOST_AUTO(pLine1, acedPick(
			bind(&id2SharedPtr<AcDbLine>, _1, AcDb::kForWrite, false), 
			TEXT("\n请点取搭接标注：")));
	BOOST_AUTO(pLine2, acedPick(
		bind(&id2SharedPtr<AcDbLine>, _1, AcDb::kForWrite, false), 
		TEXT("\n请点取搭接标注：")));

	AcGeLine3d Line1(pLine1->startPoint(), pLine1->endPoint());
	AcGeLine3d Line2(pLine2->startPoint(), pLine2->endPoint());

	if (Line1.isParallelTo(Line2))
	{
		acutPrintf(TEXT("\nline1 is colinear to line2"));
	}
	else
	{
		acutPrintf(TEXT("\nline1 is NOT colinear to line2"));
	}
}