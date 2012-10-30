#include "StdAfx.h"

#include "Util_acedget.h"
#include "Debug.h"
#include "Util_arx_cmd_helpers.h"
#include "Configurable.h"

GwaArx::Util::_aced_get::CAcEdSSGet::CAcEdSSGet()
:m_prompt(), m_str(), m_pt1(NULL), m_pt2(NULL), m_filter(NULL), m_sentry()
{
	::memset(m_ss, 0, sizeof(ads_name));	
	
	/****************************************************************
	post condition:
		all wstring type hold an empty string (i.e. L"").
		all pointer & array type got NULL.
		boost::function got an empty (i.e. m_sentry.empty() yield true).
	****************************************************************/	
}

GwaArx::Util::_aced_get::CAcEdSSGet & 
GwaArx::Util::_aced_get::CAcEdSSGet::setPrompt( const std::wstring & prompt )
{
	m_prompt = prompt;
	return *this;
}

GwaArx::Util::_aced_get::CAcEdSSGet & 
GwaArx::Util::_aced_get::CAcEdSSGet::setStr( const std::wstring & str )
{
	m_str = str;
	return *this;
}

GwaArx::Util::_aced_get::CAcEdSSGet & 
GwaArx::Util::_aced_get::CAcEdSSGet::setPt1( void * pt1 )
{
	m_pt1 = pt1;
	return *this;
}

GwaArx::Util::_aced_get::CAcEdSSGet & 
GwaArx::Util::_aced_get::CAcEdSSGet::setPt2( void * pt2 )
{
	m_pt2 = pt2;
	return *this;
}

GwaArx::Util::_aced_get::CAcEdSSGet & 
GwaArx::Util::_aced_get::CAcEdSSGet::setFilter( struct resbuf * filter )
{
	m_filter = filter;
	return *this;
}

GwaArx::Util::_aced_get::CAcEdSSGet & 
GwaArx::Util::_aced_get::CAcEdSSGet::setSentry( 
	GwaArx::Util::_aced_get::CAcEdSSGet::sentry_type sentry )
{
	m_sentry = sentry;
	return *this;
}

void GwaArx::Util::_aced_get::CAcEdSSGet::operator()( ads_name *pSS /*= NULL*/ )
{
	using GwaArx::Util::_arx_cmd_helpers::UserCanceled;
	if (!m_prompt.empty())
	{		
		acutPrintf(m_prompt.c_str());
	}

	int rt = RTERROR;
	bool bOk = false;
	do 
	{
		rt = acedSSGet((m_str.empty() ? NULL : m_str.c_str()),
			m_pt1, m_pt2, m_filter, m_ss);

		switch(rt)
		{
		case RTNORM:
			if (!m_sentry)
			{
				bOk = true;
			}	
			else if (m_sentry(m_ss))
			{
				bOk = true;
			}
			break;
		case RTERROR:
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
				"CAcEdSSGet::operator() - rejected the calling request, invalid arguemnt?");
		default:
			xssert(!"should not got here");
			throw std::runtime_error(
				"CAcEdSSGet::operator() - should not got here.");
		}
	} while (!bOk);	
	
	if (pSS)
	{
		::memcpy_s(pSS, sizeof(ads_name), m_ss, sizeof(ads_name));
	}
}


