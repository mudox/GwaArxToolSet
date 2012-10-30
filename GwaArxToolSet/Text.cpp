#include "StdAfx.h"

#include "Text.h"
#include "Util.h"

GwaArx::TextPatterns::CPatBase::entity_ptr_type 
GwaArx::TextPatterns::CPatBase::entityPtr( 
	AcDb::OpenMode mode /*= AcDb::kForRead*/, bool openErased /*= false*/ ) const
{
	using namespace GwaArx::Util;

	AcDbObjectId id;
	::curDwg()->getAcDbObjectId(id, false, m_handle);	

	return id2sp<entity_type>(id, mode, openErased);
}

std::wstring GwaArx::TextPatterns::CPatBase::Text( void ) const
{
	return std::wstring(entityPtr()->textStringConst());
}

void GwaArx::TextPatterns::CPatBase::setText( std::wstring & newText ) const
{
	GwaArx::Util::esCall(entityPtr(AcDb::kForWrite)->setTextString(newText.c_str()));
}

GwaArx::TextPatterns::CPatBase::CPatBase( const AcDbHandle & handle )
: m_handle(handle)
{
	
}

GwaArx::TextPatterns::CPatBar::CPatBar( const AcDbHandle & handle )
: CPatBase(handle)
{

}

ACHAR GwaArx::TextPatterns::CPatBar::TY() const
{
	using namespace boost::xpressive;

	static const tag_type tagTY(1);
	static const reg_type reg= repeat<0, 2>(_d) >> (tagTY = _regTY()) >> _regBarDia();
	
	match_type match;
	std::wstring text = Text();
	regex_search(text, match, reg);
	
	xssert(match);
	if (match)
	{
		return match[tagTY].str()[0];
	}
	else
	{
		throw std::runtime_error("xpressive failed to yank Bar Symbol");
	}
}

void GwaArx::TextPatterns::CPatBar::setTY( ACHAR ty ) const
{	
	using namespace boost::xpressive;

	static const reg_type reg= (s1 = repeat<0, 2>(_d)) >> _regTY() >> (s2 =  _regBarDia());
	std::wstring fmt = std::wstring(L"$1") + ty + L"$2";
	setText(boost::xpressive::regex_replace(Text(), reg, fmt));
}

GwaArx::TextPatterns::CPatMainBar::shared_ptr_type 
GwaArx::TextPatterns::CPatMainBar::newInstance( const AcDbObjectId & id )
{
	using namespace boost;

	AcDbHandle handle = _Sentry(id);
	if (handle.isNull())
	{
		return shared_ptr_type(); 
	}

	return shared_ptr<CPatMainBar>(new CPatMainBar(handle));
}

AcDbHandle 
GwaArx::TextPatterns::CPatMainBar::_Sentry( const AcDbObjectId & id ) 
{
	using namespace boost::xpressive;
	using namespace GwaArx::Util;
	using namespace std;

	// constructs a null handle, which.isNull yields true.
	AcDbHandle retHanlde(0, 0);

	do 
	{
		// id ->> shared_ptr<AcDbText>
		BOOST_AUTO(textPtr, id2sp<AcDbText>(id));
		if(!textPtr)
		{
			break;
		}

		// regex test.
		// because in regex_match(), ACHAR * can't not automatically
		// convert to std::wstirng.
		// so need to construct a temporaty std::wstring object.
		wstring text(textPtr->textStringConst()); 
		if (!regex_match(text, _regSentry()))
		{
			break;
		}

		// congratulations.		
		textPtr->getAcDbHandle(retHanlde);		
	} while (0);

	// no passed, return an empty result.
	return retHanlde; 
}

GwaArx::TextPatterns::CPatMainBar::CPatMainBar( const AcDbHandle & handle )
: CPatBar(handle)
{

}

GwaArx::TextPatterns::CPatMainBar::bar_dias_type 
GwaArx::TextPatterns::CPatMainBar::barDias( void ) const
{
	using namespace std;
	using namespace boost;
	using namespace boost::xpressive;

	wstring text = Text();

	regex_iterator_type beg(text.begin(), text.end(), _regUnit()), end;

	bar_dias_type dias;
	for (; beg != end; ++beg)
	{
		int dia = lexical_cast<bar_dias_type::value_type>(
			(*beg)[unitTag_BarDia].str());
		dias.push_back(dia);
	}

	return dias;	
}

std::pair<unsigned, unsigned> GwaArx::TextPatterns::CPatMainBar::minmaxBarDia( void ) const
{
	using namespace boost;

	BOOST_AUTO(v, barDias());
	BOOST_AUTO(mm, minmax_element(v.begin(), v.end()));

	return std::pair<unsigned, unsigned>(*mm.first, *mm.second);
}
