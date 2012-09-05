#pragma once

#include "StdAfx.h"

namespace GwaArx
{
	namespace Beam
	{
		namespace _beam_length_check
		{
			void cmdBeamLenChck( void );		

			class CBeamCounter
			{
				// typedefs.
			public:
				typedef boost::shared_ptr<CBeamCounter> shared_ptr_type;

			public:
				// factory method.
				// invoke it to begin a new calculation process.
				static shared_ptr_type acedNew( void );

			private:
				// privatize constructor
				CBeamCounter(unsigned span, unsigned halfSupport, unsigned barDia);

				// fetch calculation results.
			public:			
				unsigned SpanLength( void ) const;
				unsigned SupportHalfWidth( void ) const;
				unsigned BarDiameter( void ) const;

				unsigned Top0_25( void ) const;
				unsigned Top0_15( void ) const;
				unsigned Bottom0_08( void ) const;
				unsigned Bottom0_15( void ) const;

				// data members to hold input data.
			private:
				const unsigned m_SpanLength;
				const unsigned m_SupportHalfWidth;
				const unsigned m_BarDia;
			};
		}
	}
}