#pragma once

#include "Util_arx_cmd_helpers.h"

#ifdef MDX_DEBUG
#define xssert(expr) ((!!(expr)) || (__debugbreak(), 0))
#else
#define xssert(expr) ((void)0)
#endif

#ifdef MDX_DEBUG
#define ret_eOk(expr) do { (Acad::eOk == (expr)) || (__debugbreak(), 0); } while (0)
#else
#define ret_eOk(expr) (GwaArx::Util::esCall(expr))
#endif

#ifdef MDX_DEBUG
#define ret_RTNORM(expr) do { (RTNORM == (expr)) || (__debugbreak(), 0); } while (0)
#else
#define ret_RTNORM(expr) (GwaArx::Util::rtCall(expr))
#endif