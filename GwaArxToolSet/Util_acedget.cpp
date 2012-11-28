#include "StdAfx.h"

#include "Util.h"
#include "Configurable.h"

GwaArx::Util::_aced_get::CSSGet::CSSGet()
:m_prompt(), m_str(), m_pt1(NULL), m_pt2(NULL), m_filter(NULL), 
	m_sentry(), m_ss(NULL)
{
	
	
	/****************************************************************
	post condition:
		all wstring type hold an empty string (i.e. wstring.empty() yields true).
		all pointer & array type got NULL.
		boost::function got an empty (i.e. m_sentry.empty() yields true).
	****************************************************************/	
}

GwaArx::Util::_aced_get::CSSGet & 
GwaArx::Util::_aced_get::CSSGet::setPrompt( const std::wstring & prompt )
{
	m_prompt = prompt;
	return *this;
}

GwaArx::Util::_aced_get::CSSGet & 
GwaArx::Util::_aced_get::CSSGet::setSS( ads_name ss )
{
	xssert(ss);

	m_ss = ss;
	return *this;
}

GwaArx::Util::_aced_get::CSSGet & 
GwaArx::Util::_aced_get::CSSGet::setStr( const std::wstring & str )
{
	m_str = str;
	return *this;
}

GwaArx::Util::_aced_get::CSSGet & 
GwaArx::Util::_aced_get::CSSGet::setPt1( void * pt1 )
{
	xssert(pt1);

	m_pt1 = pt1;
	return *this;
}

GwaArx::Util::_aced_get::CSSGet & 
GwaArx::Util::_aced_get::CSSGet::setPt2( void * pt2 )
{
	xssert(pt2);

	m_pt2 = pt2;
	return *this;
}

GwaArx::Util::_aced_get::CSSGet & 
GwaArx::Util::_aced_get::CSSGet::setFilter( struct resbuf * filter )
{
	xssert(filter);

	m_filter = filter;
	return *this;
}

GwaArx::Util::_aced_get::CSSGet & 
GwaArx::Util::_aced_get::CSSGet::setSentry( 
	GwaArx::Util::_aced_get::CSSGet::sentry_type sentry )
{
	m_sentry = sentry;
	return *this;
}

void GwaArx::Util::_aced_get::CSSGet::operator()( void )
{
	using GwaArx::Util::UserCanceled;

	if (!m_prompt.empty())
	{		
		acutPrintf(m_prompt.c_str());
	}

	int rt = RTERROR;
	bool bOk = false;

#ifdef MDX_DEBUG
	resbuf rb = {NULL, RTSHORT, 0};
	short errNo = 0;
#endif

	ads_name ss;
	do 
	{						
		rt = acedSSGet((m_str.empty() ? NULL : m_str.c_str()),
			m_pt1, m_pt2, m_filter, (m_ss ? m_ss : ss));

		switch(rt)
		{
		case RTNORM:
			if (!m_sentry)
			{
				bOk = true;
			}	
			else if (m_sentry(m_ss ? m_ss : ss))
			{
				bOk = true;
			}
			break;

		case RTERROR:
#ifdef MDX_DEBUG		
			//  the "ERRNO" system variable indicates the reason for the failure.
			//  check out the "¡¶AutoLISP Developer's Guide¡·- Appendixes - AutoLISP
			//+ Error Codes" for detail information.
			ret_RTNORM(::acedGetVar(TEXT("ERRNO"), &rb));
			errNo = rb.resval.rint;		
			//xssert(!"check the ERRNO for detail");		
#else
			throw std::runtime_error("acedSSGet() failed");
#endif
			
			// user pressed space or enter
			// when nothing selected.					
			break;

		case RTCAN:
			// user pressed Esc or Ctrl + C
			// to cancel the whole command.
			throw UserCanceled();

		case RTREJ:
			xssert(!"RTREJ");
			throw std::runtime_error(
				"CSSGet::operator() - rejected the calling request, invalid arguemnt?");
		
		default:
			xssert(!"should not got here");
			throw std::runtime_error(
				"CSSGet::operator() - should not got here.");
		}
	} while (!bOk);	
	
	if (!m_ss)
	{
		// user ignore the returned selection set.
		// release selection set.
		::acedSSFree(ss);
	}

}
