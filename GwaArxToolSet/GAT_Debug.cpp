#include "StdAfx.h"

#include "GAT_Debug.h"

#ifdef MDX_DEBUG
Acad::ErrorStatus	___GwaArx___ErrorStatus___ = Acad::eOk;
int					___GwaArx___RT___ = RTNORM;
#endif

void GwaArx::Debug::acedPause( std::wstring prompt )
{
	ACHAR ignore[10];
	::acedGetString(0, prompt.c_str(), ignore);
}
