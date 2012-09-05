#pragma once

#include "StdAfx.h"

namespace GwaArx
{
	namespace TextPatterns
	{
		class CPatBase
		{	
			// typedefs.
		public:
			typedef AcDbText							entity_type;
			typedef boost::shared_ptr<entity_type>		entity_ptr_type;

			// xpressive type aliases
			typedef boost::xpressive::mark_tag			tag_type;
			typedef boost::xpressive::wsregex			reg_type;
			typedef boost::xpressive::wsmatch			match_type;
			typedef boost::xpressive::wsregex_iterator  regex_iterator_type;

			// constructors & destructor.
		protected:			
			CPatBase( const AcDbHandle & handle );

			// AcDbText'Text String access functions.
		public:
			// AcDbText::stringText();
			std::wstring Text( void ) const;
			void setText( std::wstring & newText ) const;
			
		public:
			// get a shared_ptr of AcDbText from m_handle
			inline entity_ptr_type entityPtr( AcDb::OpenMode mode =  AcDb::kForRead, 
				bool openErased = false ) const ;

			// data members.
		private:
			AcDbHandle m_handle;	
		};		

				
		class CPatBar : public CPatBase
		{
			// typedefs
		public:
			typedef std::vector<unsigned> bar_dias_type;

			// constructors & destructor.
		public:
			CPatBar( const AcDbHandle & handle );

			// component access.
		public:			
			// gets ...
			virtual ACHAR TY( void ) const;
			virtual bar_dias_type barDias( void ) const = 0;
			//virtual unsigned numGroups( void ) const = 0;

			// sets ...
			virtual void setTY( ACHAR ty ) const;
			//virtual void setBarDias( const bar_dias_type & dias ) const = 0;

			// static regex objects.
		protected:					
			static const reg_type &	_regTY( void );		
			static const reg_type &	_regBarDia( bool bStrict = false );	
			static const reg_type & _regPlus( void );					
		};


		class CPatMainBar : public CPatBar
		{
			// typedefs.
		public:
			typedef boost::shared_ptr<CPatMainBar> shared_ptr_type;	

		public:			
			CPatMainBar( const AcDbHandle & handle );

			// check to see if the AcDbText object behind the passed in id
			// hold a valid text content.
			static AcDbHandle _Sentry( const AcDbObjectId & id );

			// static regex objects.
		protected:
			static const reg_type & _regNumBars( void );	
			static const reg_type & _regLV( void );	
			// mart_tag indices defined in _regUnit.
			enum { unitTag_NumBars = 1, unitTag_TY, unitTag_BarDia, unitTag_LV };
			static const reg_type & _regUnit( void );	
			static const reg_type & _regSentry( void );


		public:
			/************************************************************************
			factory method to prompt user to pick a text object					
			and 'new' an instance of appropriate PatBase derived
			type.
			************************************************************************/ 
			static shared_ptr_type newInstance( const AcDbObjectId & id );

		public:		
			
			// gets ...
			virtual bar_dias_type barDias( void ) const;
			// .first = min, .second = max.
			virtual std::pair<unsigned, unsigned> minmaxBarDia( void ) const;
			//virtual unsigned numBars() const;

			// sets ...
			//virtual void setNumBars( unsigned numBars ) const;			
		};

		//class CPatStirrup : public CPatBar
		//{
		//public:
		//	// gets ...
		//	virtual unsigned BarSpacing() const;
		//	virtual std::wstring StirrupMode() const;

		//	// sets ...
		//	virtual void SetBarSpacing( unsigned spacing );
		//	virtual void SetStirrupMode( const std::wstring & mode );
		//};

		//class CPatSideBar : public CPatMainBar
		//{
		//public:
		//	std::wstring Suffix() const;

		//	void SetSuffix( const std::wstring & suffix );
		//};

		//// ASD standard implementation classes

		//class CPatWithSN
		//{
		//public:
		//	virtual unsigned SN() const = 0;
		//	virtual unsigned SetSN( unsigned sn ) = 0;
		//};

		//class CPatMainBarASD : public CPatMainBar, public CPatWithSN
		//{

		//};

		//class CPatStirrupASD : public CPatStirrup, public CPatWithSN
		//{

		//};

		//class CPatSideBarASD : public CPatSideBar, public CPatWithSN
		//{

		//};
	}
}