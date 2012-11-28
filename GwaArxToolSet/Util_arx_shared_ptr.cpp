#include "StdAfx.h"

#include "Util.h"
			
heap_rb_sp GwaArx::Util::_arx_shared_ptr::heap_rb2sp( struct resbuf * rb )
{
	// the only argument should not be NULL;
	xssert(rb);

	try
	{
		heap_rb_sp theSp(rb, ::acutRelRb);
		xssert(theSp);
		return theSp;
	}
	catch(...)
	{
		throw std::runtime_error("heap_rb2sp() failed with an exception");
	}				
}


heap_str_sp GwaArx::Util::_arx_shared_ptr::heap_str2sp( ACHAR * str )
{					
	// the only argument should not be NULL;
	xssert(str);

	heap_str_sp theSp;
	try
	{			
		typedef void (*D)( wchar_t *& );
		
		theSp.reset<ACHAR, D>(str, ::acutDelString);
		xssert(theSp);		
	}
	catch(...)
	{
		throw std::runtime_error("heap_str2sp() failed with an exception");
	}

	return theSp;
}

btr_sp GwaArx::Util::_arx_shared_ptr::curDbBTR( 
	std::wstring blockName /*= ACDB_MODEL_SPACE*/, 
	AcDb::OpenMode openMode /*= = AcDb::kForWrite*/ )
{
	AcDbBlockTable *pBlockTalbe;
	ret_eOk(curDwg()->getSymbolTable(pBlockTalbe,  AcDb::kForRead));

	AcDbBlockTableRecord *pBlockTableRecord;
	ret_eOk(pBlockTalbe->getAt(blockName.c_str(), pBlockTableRecord, openMode));

	pBlockTalbe->clone();

	return boost::shared_ptr<AcDbBlockTableRecord>(
		pBlockTableRecord, std::mem_fun(&AcDbBlockTableRecord::close));
}

inline heap_str_sp GwaArx::Util::_arx_shared_ptr::newHandleStrBufSp( void )
{
	//  A handle string can be up to 16 bytes long, so the buffer should be at 
	//+ least 17 bytes to handle the string and the null terminator.
	return heap_str2sp(new ACHAR[17]);
}

std::wstring GwaArx::Util::_arx_shared_ptr::toWStr( ACHAR * str )
{
	std::wstring ret(str);
	::acutDelString(str);

	return ret;
}
