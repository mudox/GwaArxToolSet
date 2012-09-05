#include "StdAfx.h"

#include "Configurable.h"
#include "Util.h"


unsigned GwaArx::Configurations::CGwaDataSheet::AnchorageLength_46D( unsigned barDia )
{
	switch(barDia)
	{
	case 16:
		return 740;
		break;
	case 20:
		return 920;
		break;
	case 25:
		return 1150;
		break;
	case 32:
		return 1480;
		break;
	case 40:
		return 1840;
		break;
	default:
		throw std::invalid_argument(
			"Invalid bar diameter, only { 16, 20, 25, 32, 40 } is allowed");
	}
}

unsigned GwaArx::Configurations::CGwaDataSheet::LappingLength_46D( unsigned barDia )
{
	GwaArx::Util::VerifyBarDia(barDia);

	unsigned ret = 0;
	switch(barDia)
	{
	case 10:
		return 500;
	case 12:
		return 600;
	case 16:
		return 750;		
	case 20:
		return 950;		
	case 25:
		return 1150;		
	case 32:
		return 1500;		
	case 40:
		return 1850;		
	default:
		throw std::runtime_error("LappingLength_46D()  should not get there");
	}
}

bool GwaArx::Configurations::CGwaDataSheet::IsValidBarDia( unsigned dia )
{
	switch(dia)
	{
	case 10:	
	case 12:	
	case 16:		
	case 20:		
	case 25:	
	case 32:
	case 40:
		return true;
	default:
		return false;
	}
}

boost::filesystem::wpath GwaArx::Configurations::CConfigurable::cfgRoot()
{
	using namespace boost::filesystem;


	static wpath _cfgRoot;
	
	if (_cfgRoot.empty())
	{
		ACHAR _modulePath[MAX_PATH] = { 0 };
		DWORD dwRet = GetModuleFileName(_hdllInstance, _modulePath, MAX_PATH);
		if (0 == dwRet || MAX_PATH == dwRet)
		{
			throw std::runtime_error("GetModuleFileName() failed");
		}

		_cfgRoot = _modulePath;	
		_cfgRoot.remove_filename();
		_cfgRoot /= TEXT("Configurables");	
	}

	return _cfgRoot;
}

std::wstring GwaArx::Configurations::CWcPatterns::_composePatText( std::wstring cfgFileName )
{
	using namespace std;

	using namespace boost::filesystem;
	namespace bfs = boost::filesystem;
	using namespace boost::xpressive;	

	using namespace GwaArx::Util;

	// compose file name.
	wpath cfgPath = (cfgRoot() /= cfgFileName);
	if (!exists(cfgPath))
	{			
		throw std::runtime_error("file \"" + string_cast<30>(cfgFileName)
			+ "\" does not exists");
	}

	// open file.
	bfs::wifstream cfgFile(cfgPath);
	if (!cfgFile)
	{
		throw std::runtime_error("failed to open Wc Pattern file \"" 
			+ string_cast<30>(cfgFileName));
	}

	// read all file content.
	wstring got(
		istreambuf_iterator<wchar_t>(cfgFile.rdbuf()), 
		istreambuf_iterator<wchar_t>());

	// parse file content to compose the final pattern text.

	// remove heading spaces & empty lines.
	got = regex_replace(got, 
		wsregex(bos >> +_s), 
		L""); 		
	// trim_left & trim_right for every remaining lines.
	got = regex_replace(got, 
		wsregex(*blank >> +_n >> *blank), 
		L"\n"); 
	// concatenate all line into one line, L',' as delimiter.
	got = regex_replace(got, 
		wsregex(_n), 
		L","); 
	// remove comment lines.
	got = regex_replace(got,
		wsregex(L"//" >> -*_ >> L',' ), 
		L"");
	// remove the trailing L','.
	got.resize(got.size() - 1); 	

	return got;
}

std::wstring GwaArx::Configurations::CWcPatterns::WcMainBar( void )
{
	return _composePatText(L"WcMainBar.cfg");	
}

std::wstring GwaArx::Configurations::CWcPatterns::WcSideBar( void )
{
	return _composePatText(L"WcSidebar.cfg");	
}

std::wstring GwaArx::Configurations::CWcPatterns::WcStirrup( void )
{
	return _composePatText(L"WcStirrup.cfg");	
}	

std::wstring GwaArx::Configurations::CWcPatterns::WcSpaceBar( void )
{
	return _composePatText(L"WcSpaceBar.cfg");	
}


