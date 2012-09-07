#include "StdAfx.h"

int main()
{
	using namespace boost::filesystem;
	namespace bfs = boost::filesystem;
	using namespace boost::xpressive;
	using namespace std;
	
	try
	{
		wpath cfgPath(L"D:/MDX.RAIN/Dropbox/AUTOCAD.DROP/AP/Configurables/WcMainBar.cfg");

		if (!exists(cfgPath))
		{
			std::cout << "missing file" << std::endl;
			throw 1;
		}

		bfs::wifstream cfgFile(cfgPath);
		if (!cfgFile)
		{
			std::cout << "filing opening failed" << std::endl;
			throw 1;
		}

 		wstring got(
 			istreambuf_iterator<wchar_t>(cfgFile.rdbuf()), 
 			istreambuf_iterator<wchar_t>());

		wsregex aSapce = as_xpr(L' ');
		wsregex aTab = as_xpr(L'\t');
		wsregex aNewLine = as_xpr(L'\n');

		wstring before;
		before = regex_replace(got, aSapce, L"[ ]");
		before = regex_replace(before, aTab, L"[\\t]");
		before = regex_replace(before, aNewLine, L"[\\n]");

		std::wcout << got.size() << std::endl;
		std::wcout << got << std::endl;

		// +++++++++++

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

		// +++++++++++

		wstring after;
		after = regex_replace(got, aSapce, L"[ ]");
		after = regex_replace(after, aTab, L"[\\t]");
		after = regex_replace(after, aNewLine, L"[\\n]");

		std::wcout << got.size() << std::endl;
		std::wcout << got << std::endl;
		
		
		
	}
	catch(...)
	{
		std::cout << "program failed" << std::endl;
	}

	system("pause");
	return 0;
}