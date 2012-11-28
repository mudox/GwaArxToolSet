#pragma once

#include "StdAfx.h"

namespace GwaArx
{
	namespace Util
	{
		namespace _misc
		{
			void verifyBarDia( unsigned dia );	
			//unsigned rbCount( resbuf *rb, short key, resbuf val);

			AcDbObjectId name2id( const ads_name name );

			heap_str_sp id2handleStrBufSp( AcDbObjectId id );
		
			template<typename OutputIterator>
			void ss2Ids ( const ads_name ss,  OutputIterator outIter )
			{
				using namespace GwaArx::Util;

				long ssLen;
				ret_RTNORM(::acedSSLength(ss, &ssLen));

				ads_name name;
				for(long n = 0; n != ssLen; ++n)
				{
					ret_RTNORM(::acedSSName(ss, n, name));
					*outIter++ = name2id(name);
				}
			}

			void ssHighlight( const ads_name ss );	
			template<typename InputIterator>
			void idsHighlight(InputIterator beg, InputIterator end)
			{				
				for (InputIterator iter = beg; iter != end; ++iter)
				{
					GwaArx::Util::id2sp<AcDbEntity>(*iter)->highlight();
				}
			}
		}
	}
}
