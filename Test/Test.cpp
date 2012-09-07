// Test.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <gepnt2d.h>

#define kPi (3.14159)

int _tmain(int argc, _TCHAR* argv[])
{
	
	AcGeMatrix2d mat;
	mat.setToRotation( kPi / 2);

	AcGePoint2d pnt(10.0, 20.0);

	pnt.transformBy(mat);


	system("pause");
	return 0;
}

