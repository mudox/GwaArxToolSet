#include "StdAfx.h"

#include "Util.h"
			
rb_sp GwaArx::Util::_arx_shared_ptr::rb2sp( struct resbuf * rb )
{
	// the only argument should not be NULL;
	xssert(rb);

	try
	{
		rb_sp theSp(rb, ::acutRelRb);
		xssert(theSp);
		return theSp;
	}
	catch(...)
	{
		throw std::runtime_error("rb2sp() failed with an exception");
	}				
}


acharbuf_sp GwaArx::Util::_arx_shared_ptr::achar2sp( ACHAR * str )
{					
	// the only argument should not be NULL;
	xssert(str);

	try
	{			
		typedef void (*D)( wchar_t *& );
		acharbuf_sp theSp;
		theSp.reset<ACHAR, D>(str, ::acutDelString);
		xssert(theSp);
		return theSp;
	}
	catch(...)
	{
		throw std::runtime_error("achar_sp() failed with an exception");
	}			
}

btr_sp GwaArx::Util::_arx_shared_ptr::openBlockTableRecord( 
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
