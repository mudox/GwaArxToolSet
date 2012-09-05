#pragma once

#include "StdAfx.h"
#include "Util.h"

namespace GwaArx
{
	namespace Util	
	{
		namespace _arx_shared_ptr
		{
			template<typename T_Object> 
			boost::shared_ptr<T_Object> id2SharedPtr(
				const AcDbObjectId & id, 
				AcDb::OpenMode mode =  AcDb::kForRead, 
				bool openErased = false)
			{
				T_Object * p = NULL;
				Acad::ErrorStatus es = acdbOpenObject(p, id, mode, openErased);
				if( Acad::eOk == es )
				{
					return boost::shared_ptr<T_Object>(p, std::mem_fun(&T_Object::close));	
				}
				else
				{	
					return boost::shared_ptr<T_Object>();
				}
			}

			template<typename T_Object> 
			boost::shared_ptr<T_Object> name2SharedPtr(
				ads_name sname, 
				AcDb::OpenMode mode =  AcDb::kForRead, 
				bool openErased = false)
			{
				// ads_name ->> acdbObjectId
				AcDbObjectId id;
				ret_eOk(acdbGetObjectId(id, sname));

				// acdbObjectId ->> shared_ptr<T_Object>
				return id2SharedPtr<T_Object>(id, mode, openErased);
			}
		}

	}
}