#include "StdAfx.h"

#include "Util.h"

void GwaArx::Util::_arx_cmd_helpers::InvokeCmdImp( CmdImpFunc pfunc )
{
	using namespace GwaArx::Util;

	try
	{
		pfunc();
	}
	catch(UserCanceled)
	{
		// nothing to do, just return
	}
	catch(std::exception &e)
	{
		int rt = acutPrintf(TEXT("*Error: %s*\n"), string_cast<200>(e.what()));
		xssert(RTNORM == rt);		
	}
} // InvokeCmdImp()

int GwaArx::Util::_arx_cmd_helpers::rtCall( int rt )
{
	if (RTNORM == rt)
	{
		return rt;
	}

	std::string errInfo;
	switch (rt)
	{
	case RTERROR:
		errInfo = "rtCall() - ads function invocation failed";
		break;
	case RTREJ:
		errInfo = "rtCall() - request rejected";
		break;
	case RTCAN:
		throw UserCanceled();
	case RTKWORD:
	case RTNONE:
		return rt;
	default:
		errInfo = "rtCall() - should not got here";
	}

	throw std::runtime_error(errInfo);
} // RtCall()

void GwaArx::Util::_arx_cmd_helpers::esCall( Acad::ErrorStatus err )
{
	if (Acad::eOk == err)
	{
		return;
	}

	std::string errInfo;
	switch(err)
	{
	case Acad::eInvalidAdsName:
		errInfo = "esCall - invalid ads_name value)";
		break;
	case Acad::eUnknownHandle:
 		errInfo = "esCall - invalid AcDbHanlde value";
	default:	
		errInfo = "esCall - not returning Acad::eOk";
	}

	throw std::runtime_error(errInfo);
} // EsCall()