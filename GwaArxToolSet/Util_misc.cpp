#include "StdAfx.h"

#include "Util.h"
#include "Configurable.h"

#include <strsafe.h>

void GwaArx::Util::_misc::verifyBarDia( unsigned dia )
{	
	if (!GwaArx::Configurations::CGwaDataSheet::IsValidBarDia(dia))
	{
		throw std::invalid_argument(
			"无效的钢筋直径, 只允许 { 10, 12, 16, 20, 25, 32, 40 }");
	}	
}

AcDbObjectId GwaArx::Util::_misc::name2id( const ads_name name )
{
	AcDbObjectId id;
	ret_eOk(::acdbGetObjectId(id, name));
	return id;
}

heap_str_sp GwaArx::Util::_misc::id2handleStrBufSp( AcDbObjectId id )
{
	BOOST_AUTO(sp, id2sp<AcDbObject>(id));
	xssert(sp);

	AcDbHandle handle;
	sp->getAcDbHandle(handle);	
	xssert(!handle.isNull());
	
	heap_str_sp handleStrBufSp = heap_str2sp(new ACHAR[17]);
	handle.getIntoAsciiBuffer(handleStrBufSp.get());

	return handleStrBufSp;
}

void GwaArx::Util::_misc::ssHighlight( const ads_name ss )
{
	long ssLen;
	ret_RTNORM(::acedSSLength(ss, &ssLen));

	ads_name name;
	for (int n = 0; n != ssLen; ++n)
	{		
		ret_RTNORM(::acedSSName(ss, n, name));
		ret_RTNORM(::acedRedraw(name, 3));
	}
}