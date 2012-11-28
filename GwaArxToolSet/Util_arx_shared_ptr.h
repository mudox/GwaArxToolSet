#pragma once

#include "StdAfx.h"

#include "Util.h"

namespace GwaArx 
{
	namespace Util 
	{
		namespace _arx_shared_ptr 
		{
			template<typename AcDbObj_T>
			boost::shared_ptr<AcDbObj_T> dbptr2sp( AcDbObj_T * p )
			{		
				// p must point to a object of or derived from class AcDbObject
				xssert(AcDbObject::cast(p));

				boost::shared_ptr<AcDbObj_T> sp;
				try
				{
					sp.reset(p, std::mem_fun(&AcDbObj_T::close));
				}
				catch(...)
				{
					throw std::runtime_error("contrcucting boost::shared_ptr failed");
				}				

				xssert(sp);
				return sp;
			}			

			//	apply acdbOpenOject() to acdbObjectId argument, if succeeded, wrap the returned
			//+ pointer which refers to the underlying acdbOject descendant in boost::shared_ptr,
			//+ else return an empty boost::shared_ptr ( which in boolean context, evaluates to 
			//+ false )
			//  the function can be used as sentry for GwaArx::Util::acedPick()
			template<typename AcDbObj_T> 
			boost::shared_ptr<AcDbObj_T> id2sp(
				const AcDbObjectId &	id, 
				AcDb::OpenMode			openMode	=  AcDb::kForRead, 
				bool					openErased	= false)
			{
				AcDbObj_T * p = NULL;
				Acad::ErrorStatus es = acdbOpenObject(p, id, openMode, openErased);
				if( Acad::eOk == es && p)
				{ 						
					return dbptr2sp<AcDbObj_T>(p);
				}
				else
				{	
					// return a null ptr on failure.
					return boost::shared_ptr<AcDbObj_T>(); 
				}
			}

			//  convert ads_name to acdbOjectId, and then leave the remaining work to id2sp
			//+ defined ahead.
			template<typename AcDbObj_T> 
			boost::shared_ptr<AcDbObj_T> name2sp(
				const ads_name		name, 
				AcDb::OpenMode		openMode	=  AcDb::kForRead, 
				bool				openErased	= false)
			{
				// ads_name ->> acdbObjectId
				AcDbObjectId id;
				Acad::ErrorStatus es = acdbGetObjectId(id, name);
				if (Acad::eOk == Acad::eOk && id != AcDbObjectId::kNull)
				{
					// acdbObjectId ->> shared_ptr<T_Object>
					return id2sp<AcDbObj_T>(id, openMode, openErased);
				}
				else
				{
					// return null ptr in failure.					
					return boost::shared_ptr<AcDbObj_T>();					
				}			
			}

			template<typename AcDbObj_T>
			void dbSpClose(boost::shared_ptr<AcDbObj_T> &spRef)
			{
				xssert(spRef && spRef.unique() && spRef->database());
				spRef.reset();
			}
					
			heap_rb_sp heap_rb2sp( resbuf * rb );			
			
			//  Many ARX APIs internally allocate a string buffer in heap, and then 
			//+ return the address of the buffer. the invoker are responsible for the
			//+ reclaiming of the returned buffer.
			//  So this function automates the memory collection of the returned 
			//+ buffers by delegate them to boost::shared_ptr with a custom deleter.
			
			std::wstring toWStr( ACHAR * str );
			heap_str_sp heap_str2sp( ACHAR * str );		
			heap_str_sp newHandleStrBufSp( void );
			
			//  open current dwg file's database, and obtain the specified block table
			//+ table.
			btr_sp curDbBTR( 
				std::wstring blockName = ACDB_MODEL_SPACE, 
				AcDb::OpenMode openMode = AcDb::kForWrite);			
		} // namespace _arx_shared_ptr
	} // namespace Util
} // namespace GwaArx