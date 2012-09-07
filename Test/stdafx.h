// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#pragma warning(disable : 4996)

#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <stdio.h>
#include <tchar.h>

#include <intrin.h>

// stl
#include <iostream>
#include <functional>
#include <cassert>

// boost
#include <boost/smart_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

//arx
#include <gemat2d.h>
#include <gevec2d.h>
#include <gepent2d.h>
