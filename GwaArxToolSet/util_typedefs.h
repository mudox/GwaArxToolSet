#pragma once

#include "StdAfx.h"

#define def_sp(x) boost::shared_ptr< x >

typedef def_sp(AcDbEntity)			 	dbent_sp;
typedef def_sp(AcDbObject)			 	dbobj_sp;
typedef def_sp(AcDbLine)				dbline_sp;
typedef def_sp(AcDbArc)					dbarc_sp;			
typedef def_sp(AcDbRotatedDimension)	dbdim_sp;

typedef def_sp(AcDbBlockTable)			bt_sp;
typedef def_sp(AcDbBlockTableRecord)	btr_sp;

typedef def_sp(resbuf)					rb_sp;	
typedef def_sp(ACHAR)					acharbuf_sp;


#undef def_sp

struct ent_name 
{
#ifndef _WIN64
typedef __w64 long data_t;
#else	
	typedef	__int64 data_t;
#endif
	
	explicit ent_name(ads_name name)		
		: name_0(name[0]), name_1(name[1])
	{
		
	}

	// implicitly converts to __int64, where ads_name is required.
	operator data_t *() 
	{
		return (data_t *)this;
	}

	data_t name_0;
	data_t name_1;
	
};






