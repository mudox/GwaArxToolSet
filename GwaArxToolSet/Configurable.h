#pragma once

#include "StdAfx.h"

namespace GwaArx
{
	namespace Configurations
	{
		class CConfigurable
		{
		public:
			static boost::filesystem::wpath cfgRoot();
		};

		class CGwaDataSheet : public CConfigurable
		{
		public:
			static unsigned AnchorageLength_46D(unsigned barDia);
			static unsigned LappingLength_46D(unsigned barDia);
			static bool IsValidBarDia(unsigned dia);
		};
	
		class CWcPatterns : public CConfigurable
		{
		public:
			static std::wstring WcMainBar( void );
			static std::wstring WcSideBar( void );
			static std::wstring WcStirrup( void );
			static std::wstring WcSpaceBar( void );	

		private:
			static std::wstring _composePatText( std::wstring cfgFileName );
		};		
	}
}