// (C) Copyright 2002-2005 by Autodesk, Inc. 
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted, 
// provided that the above copyright notice appears in all copies and 
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting 
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS. 
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC. 
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to 
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//

//-----------------------------------------------------------------------------
//----- acrxEntryPoint.h
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "resource.h"

#include "Util.h"
#include "UnitTest.hpp"
#include "Beam.h"

//-----------------------------------------------------------------------------
#define szRDS _RXST("MDX")

//-----------------------------------------------------------------------------
//----- ObjectARX EntryPoint
class CGwaArxToolSetApp : public AcRxArxApp {

public:
	CGwaArxToolSetApp () : AcRxArxApp () 
	{

	}

	virtual AcRx::AppRetCode On_kInitAppMsg (void *pkt) 
	{
		// TODO: Load dependencies here

		// You *must* call On_kInitAppMsg here
		AcRx::AppRetCode retCode =AcRxArxApp::On_kInitAppMsg (pkt) ;
		
		// TODO: Add your initialization code here		

		return (retCode) ;
	}

	virtual AcRx::AppRetCode On_kUnloadAppMsg (void *pkt) 
	{
		// TODO: Add your code here

		// You *must* call On_kUnloadAppMsg here
		AcRx::AppRetCode retCode =AcRxArxApp::On_kUnloadAppMsg (pkt) ;

		// TODO: Unload dependencies here

		return (retCode) ;
	}

	virtual void RegisterServerComponents ()
	{

	}
// ++++++++++++++++++++++++++ ARX Commands ++++++++++++++++++++++++++++++++++++
public:
#ifdef MDX_DEBUG
	// ----- MDX_GwaArxToolSet_.ts command
	static void MDX_GwaArxToolSet_ts(void)
	{
		JustaTest();				
	}
#endif	

	// ----- MDX_GwaArxToolSet_.TiaoZhengDaJie command (do not rename)
	static void MDX_GwaArxToolSet_TiaoZhengDaJie(void)
	{
		GwaArx::Util::InvokeCmdImp(GwaArx::Beam::cmdAdjustLapping);
	}

	// ----- MDX_GwaArxToolSet_.YanSuanLiangChang command (do not rename)
	static void MDX_GwaArxToolSet_YanSuanLiangChang(void)
	{
		GwaArx::Util::InvokeCmdImp(GwaArx::Beam::cmdCheckLength);
	}

	// - MDX_GwaArxToolSet_.WanJin command (do not rename)
	static void MDX_GwaArxToolSet_WanJin(void)
	{
		GwaArx::Util::InvokeCmdImp(GwaArx::Beam::cmdBendABar);
	}

	// - MDXGwaArxToolSet_.ZiDongWanJin command (do not rename)
	static void MDXGwaArxToolSet_ZiDongWanJin(void)
	{
		GwaArx::Util::InvokeCmdImp(GwaArx::Beam::cmdBendBarsAuto);
	}
};

//-----------------------------------------------------------------------------
IMPLEMENT_ARX_ENTRYPOINT(CGwaArxToolSetApp)

#ifdef MDX_DEBUG
ACED_ARXCOMMAND_ENTRY_AUTO(CGwaArxToolSetApp,MDX_GwaArxToolSet_, ts, ts,ACRX_CMD_TRANSPARENT | ACRX_CMD_USEPICKSET | ACRX_CMD_REDRAW,NULL)
#endif

ACED_ARXCOMMAND_ENTRY_AUTO(CGwaArxToolSetApp, MDX_GwaArxToolSet_, TiaoZhengDaJie, TiaoZhengDaJie, ACRX_CMD_TRANSPARENT | ACRX_CMD_USEPICKSET | ACRX_CMD_REDRAW, NULL)
ACED_ARXCOMMAND_ENTRY_AUTO(CGwaArxToolSetApp, MDX_GwaArxToolSet_, YanSuanLiangChang, YanSuanLiangChang, ACRX_CMD_TRANSPARENT, NULL)
ACED_ARXCOMMAND_ENTRY_AUTO(CGwaArxToolSetApp, MDX_GwaArxToolSet_, WanJin, WanJin, ACRX_CMD_TRANSPARENT, NULL)
ACED_ARXCOMMAND_ENTRY_AUTO(CGwaArxToolSetApp, MDXGwaArxToolSet_, ZiDongWanJin, ZiDongWanJin, ACRX_CMD_TRANSPARENT, NULL)
