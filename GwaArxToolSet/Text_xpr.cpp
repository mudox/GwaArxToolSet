#include "StdAfx.h"

#include "Text.h"
#include "Util.h"

#define COMMON_PREFIX const GwaArx::TextPatterns::CPatBase::reg_type & GwaArx::TextPatterns

COMMON_PREFIX::CPatBar::_regTY( void )
{
	using namespace boost::xpressive;

	static const reg_type _wsre = (set = L'T', L't', L'Y', L'y');					
	return _wsre;	
}

COMMON_PREFIX::CPatBar::_regBarDia( bool bStrict /*= false*/ )
{
	using namespace boost::xpressive;

	static const reg_type _wsre1 = _d >> _d;		// 2 digits
	static const reg_type _wsre2 = (set = L'10', L'12', L'16', L'20', L'25', L'32');		
	return bStrict ? _wsre2 : _wsre1;	
}

COMMON_PREFIX::CPatBar::_regPlus( void )
{
	using namespace boost::xpressive;

	static const reg_type _wsre = *blank >> L'+' >> *blank; 				
	return _wsre;
}

COMMON_PREFIX::CPatMainBar::_regNumBars( void )
{
	using namespace boost::xpressive;

	static const reg_type _wsre = repeat<1, 2>(_d);	

	return _wsre;
}

COMMON_PREFIX::CPatMainBar::_regLV( void )
{
	using namespace boost::xpressive;

	static const reg_type _wsre = L'(' >> *blank >> 
		(set = L'L', L'l') >> *blank >>
		L'.' >> *blank >>
		(set = L'V', L'v') >> *blank >>
		L'.' >> *blank >>
		L')'; 				

	return _wsre;
}

COMMON_PREFIX::CPatMainBar::_regUnit( void )
{
	using namespace boost::xpressive;

	static const reg_type _wsre = (mark_tag(unitTag_NumBars) = _regNumBars()) 
		>> (mark_tag(unitTag_TY) = _regTY()) 
		>> (mark_tag(unitTag_BarDia) = _regBarDia())
		>> (mark_tag(unitTag_LV) = !(*blank >> _regLV()));
	
	return _wsre;
}

COMMON_PREFIX::CPatMainBar::_regSentry( void )
{
	using namespace boost::xpressive;

	static const reg_type _wsre = _regUnit() 
		>> *(_regPlus() >> _regUnit());

	return _wsre;
}

#undef  COMMON_PREFIX

