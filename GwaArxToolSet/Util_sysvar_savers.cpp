#include "StdAfx.h"

#include "Util_sysvar_savers.h"

GwaArx::Util::_sysvar_savers::
CursorStatusAutoRecorver::CursorStatusAutoRecorver(
	const sysvar_bit which ) : m_which(which)
{
	resbuf rb = { NULL };
	rb.restype = RTSHORT;

	// save "OSMODE"
	if (which & setOSMODE)
	{
		::acedGetVar(TEXT("OSMODE"), &rb);
		m_osMode = rb.resval.rint;
	}

	// save "SNAPMODE"
	if (which & setSNAPMODE)
	{
		::acedGetVar(TEXT("SNAPMODE"), &rb);
		m_snapMode = rb.resval.rint;
	}

	// save "ORTHOMODE"
	if (which & setORTHOMODE)
	{
		::acedGetVar(TEXT("ORTHOMODE"), &rb);
		m_orthoMode = rb.resval.rint;
	}
}

GwaArx::Util::_sysvar_savers::
CursorStatusAutoRecorver::~CursorStatusAutoRecorver()
{
	resbuf rb = { NULL };
	rb.restype = RTSHORT;

	// save "OSMODE"
	if (m_which & setOSMODE)
	{	
		rb.resval.rint = m_osMode;
		::acedSetVar(TEXT("OSMODE"), &rb);
	}

	// save "SNAPMODE"
	if (m_which & setSNAPMODE)
	{
		rb.resval.rint = m_snapMode;
		::acedSetVar(TEXT("SNAPMODE"), &rb);
	}

	// save "ORTHOMODE"
	if (m_which & setORTHOMODE)
	{
		rb.resval.rint = m_orthoMode;
		::acedSetVar(TEXT("ORTHOMODE"), &rb);
	}
}

GwaArx::Util::_sysvar_savers::
FreeCursorAWhile::FreeCursorAWhile( 
	const sysvar_bit & which ) : CursorStatusAutoRecorver(which)
{
	//  base class CursorStatusAutoRecorver's constructor will
	//+ will run first which in turn will backup the old settings
	//+ for subsequence recovering.

	resbuf rb = { NULL };
	rb.restype = RTSHORT;

	// save "OSMODE"
	if (which & setOSMODE)
	{	
		rb.resval.rint = osNULL;
		::acedSetVar(TEXT("OSMODE"), &rb);
	}

	// save "SNAPMODE"
	if (which & setSNAPMODE)
	{	
		rb.resval.rint = 0;
		::acedSetVar(TEXT("SNAPMODE"), &rb);
	}

	// save "ORTHOMODE"
	if (which & setORTHOMODE)
	{
		rb.resval.rint = 0;
		::acedSetVar(TEXT("ORTHOMODE"), &rb);
	}
}

GwaArx::Util::_sysvar_savers::
FreeCursorAWhile::~FreeCursorAWhile()
{
	//  nothing need to do
	//  the base class CursorStatusAutoRecorver's destructor
	//+ will recover the saved settings.
}
