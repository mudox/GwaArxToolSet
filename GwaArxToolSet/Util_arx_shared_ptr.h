#pragma once

#include "StdAfx.h"

#include "Util.h"

namespace GwaArx 
{
	namespace Util 
	{
		namespace _arx_shared_ptr 
		{
			//	apply acdbOpenOject() to acdbObjectId argument, if succeeded, wrap the returned
			//+ pointer which refers to the underlying acdbOject descendant in boost::shared_ptr,
			//+ else return an empty boost::shared_ptr ( which in boolean context, evaluates to 
			//+ false )
			//  the function can be used as sentry for GwaArx::Util::acedPick()
			template<typename T_Object> 
			boost::shared_ptr<T_Object> id2sp(
				const AcDbObjectId & id, 
				AcDb::OpenMode mode =  AcDb::kForRead, 
				bool openErased = false)
			{
				T_Object * p = NULL;
				Acad::ErrorStatus es = acdbOpenObject(p, id, mode, openErased);
				if( Acad::eOk == es && p)
				{
					return boost::shared_ptr<T_Object>(p, 
						std::mem_fun(&T_Object::close));	
				}
				else
				{	
					// return a null ptr in failure					
					return boost::shared_ptr<T_Object>(); 
				}
			}

			//  convert ads_name to acdbOjectId, and then leave the remaining work to id2sp
			//+ defined ahead.
			template<typename T_Object> 
			boost::shared_ptr<T_Object> name2sp(
				const ads_name		sname, 
				AcDb::OpenMode		mode		=  AcDb::kForRead, 
				bool				openErased	= false)
			{
				// ads_name ->> acdbObjectId
				AcDbObjectId id;
				Acad::ErrorStatus es = acdbGetObjectId(id, sname);
				if (Acad::eOk == Acad::eOk && id != AcDbObjectId::kNull)
				{
					// acdbObjectId ->> shared_ptr<T_Object>
					return id2sp<T_Object>(id, mode, openErased);
				}
				else
				{
					// return null ptr in failure.					
					return boost::shared_ptr<T_Object>();					
				}			
			}
					
			rb_sp rb2sp( resbuf * rb );			
			
			//  many ARX APIs internally allocate a string buffer in heap, and then return the 
			//+ address of the buffer. the invoker are responsible for the reclaiming of the 
			//+ returned buffer.
			//  so this function automate the memory collection of the returned buffers by delegate
			//+ them to boost::shared_ptr with a custom deleter.
			
			acharbuf_sp achar2sp( ACHAR * str );				
			
			btr_sp openBlockTableRecord( 
				std::wstring blockName = ACDB_MODEL_SPACE, 
				AcDb::OpenMode openMode = AcDb::kForWrite);
		} // namespace _arx_shared_ptr
	} // namespace Util
} // namespace GwaArx