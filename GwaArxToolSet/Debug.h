#pragma once

#include "Util_arx_cmd_helpers.h"

// when configuring debug version of project property, make sure to set /D "MDX_DEBUG

#ifdef MDX_DEBUG
#define xssert(expr) ((!!(expr)) || (__debugbreak(), 0))
#else
#define xssert(expr)
#endif

#ifdef MDX_DEBUG
extern Acad::ErrorStatus ___GwaArx___ErrorStatus___;
#define ret_eOk(expr) do { (___GwaArx___ErrorStatus___ = (expr), \
	Acad::eOk == ___GwaArx___ErrorStatus___) || (__debugbreak(), 0); } while (0)
#else
#define ret_eOk(expr) (GwaArx::Util::esCall(expr))
#endif

#ifdef MDX_DEBUG
extern int ___GwaArx___RT___;
#define ret_RTNORM(expr) do { (___GwaArx___RT___ = (expr), \
	RTNORM == ___GwaArx___RT___) || (__debugbreak(), 0); } while (0)
#else
#define ret_RTNORM(expr) (GwaArx::Util::rtCall(expr))
#endif

#ifdef MDX_DEBUG
#define MDX_DBG(fn) do { (fn); } while (0)
#else
#define MDX_DBG(fn)
#endif

#ifdef MDX_DBG
#define MDX_PAUSE_HERE(prompt) do { GwaArx::Debug::acedPause(prompt); } while (0)
#else
#define MDX_PAUSE_HERE(prompt)
#endif

namespace GwaArx
{
	namespace Debug
	{
		void acedPause(std::wstring pompt);
	}
}