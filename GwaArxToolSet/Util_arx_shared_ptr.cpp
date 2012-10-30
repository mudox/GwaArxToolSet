#include "StdAfx.h"

#include "Util.h"
			
GwaArx::Util::_arx_shared_ptr::resbuf_sp 
GwaArx::Util::_arx_shared_ptr::rb2sp( struct resbuf * rb )
{
	// the only argument should not be NULL;
	xssert(rb);

	try
	{
		resbuf_sp theSp(rb, ::acutRelRb);
		xssert(theSp);
		return theSp;
	}
	catch(...)
	{
		throw std::runtime_error("rb2sp() failed with an exception");
	}				
}


GwaArx::Util::_arx_shared_ptr::achar_sp 
GwaArx::Util::_arx_shared_ptr::achar2sp( ACHAR * str )
{					
	// the only argument should not be NULL;
	xssert(str);

	try
	{			
		typedef void (*D)( wchar_t *& );
		achar_sp theSp;
		theSp.reset<ACHAR, D>(str, ::acutDelString);
		xssert(theSp);
		return theSp;
	}
	catch(...)
	{
		throw std::runtime_error("achar_sp() failed with an exception");
	}			
}