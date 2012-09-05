#include "StdAfx.h"

#include "Util.h"
#include "Configurable.h"
#include "Text.h"
#include "Beam.h"

unsigned GwaArx::Beam::_beam_length_check::CBeamCounter::Top0_25( void ) const
{
	unsigned ret = (std::ceil((m_SpanLength * 0.25 + m_SupportHalfWidth) / 100)) * 100;
	unsigned _46d = GwaArx::Configurations::CGwaDataSheet::LappingLength_46D(m_BarDia);
	ret = (ret > _46d) ? ret : _46d;
	return ret;
}

unsigned GwaArx::Beam::_beam_length_check::CBeamCounter::Top0_15( void ) const
{
	unsigned ret = (std::ceil((m_SpanLength * 0.15 + m_SupportHalfWidth) / 100)) * 100;
	unsigned _46d = GwaArx::Configurations::CGwaDataSheet::LappingLength_46D(m_BarDia);
	ret = (ret > _46d) ? ret : _46d;
	return ret;
}

unsigned GwaArx::Beam::_beam_length_check::CBeamCounter::Bottom0_08( void ) const
{
	// round off to multiple of 100
	return (int(m_SpanLength * 0.08) / 100) * 100;
}

unsigned GwaArx::Beam::_beam_length_check::CBeamCounter::Bottom0_15( void ) const
{
	// round off to multiple of 100
	return (int(m_SpanLength * 0.15) / 100) * 100;	
}

GwaArx::Beam::_beam_length_check::CBeamCounter::CBeamCounter( 
	unsigned span, unsigned halfSupport, unsigned barDia )
: m_SpanLength(span), m_SupportHalfWidth(halfSupport), m_BarDia(barDia)
{
	GwaArx::Util::VerifyBarDia(barDia);
}

namespace
{
	inline unsigned _getBarDia( void )
	{		
		using namespace GwaArx::Util;
		using namespace GwaArx::TextPatterns;
		
		int dia = 0;		
		
		BOOST_AUTO(pMainBar, acedPickA<CPatMainBar>(TEXT("请选择主筋文字：")));
		xssert(pMainBar);		
		
		return pMainBar->minmaxBarDia().second;
	}


	inline unsigned _getBeamSpanLength( void )
	{
		using namespace GwaArx::Util;	

		ads_name dim;
		ads_point ptIgnore;
		AcDbObjectId dimID;
		AcDbRotatedDimension *pRDim = NULL;
		
		do 
		{
			switch(acedEntSel(TEXT("\n点选梁跨标注："), dim, ptIgnore))
			{
			case RTERROR:				
				continue;
			case RTCAN:
				throw UserCanceled();			
			}

			esCall(acdbGetObjectId(dimID, dim));		
			if(Acad::eOk != acdbOpenObject(pRDim, dimID, AcDb::kForRead))
			{				
				acutPrintf(TEXT("\n必须是标注对象。"));
				continue;
			}
			else
			{
				break;
			}
		} while (1);
		
		unsigned ret = std::abs(std::ceil(pRDim->xLine1Point().x - pRDim->xLine2Point().x));

		pRDim->close();
		return ret;			
	}
	
	inline unsigned _getSupportHalfWidth( void )
	{
		using namespace GwaArx::Util;

		ads_real hw = 0;
		acedInitGet(RSG_NONEG, NULL);		
		int rt = acedGetDist(NULL, TEXT("\n支座半宽 <0>："), &hw);
		switch(rt)
		{
		case RTNORM:	// user inputed a invalid number			
		case RTNONE:	// user pressed enter or space to accept the default value
			break;
		case RTCAN:
			throw UserCanceled();	
		default:
			xssert(!"should not be there!");
			throw std::runtime_error("should not be there!");
		}

		return std::ceil(hw);		
	}
}

GwaArx::Beam::_beam_length_check::CBeamCounter::shared_ptr_type
GwaArx::Beam::_beam_length_check::CBeamCounter::acedNew( void )
{
	int barDia = _getBarDia();
	int spanLen = _getBeamSpanLength();
	int supportHalfWidth = _getSupportHalfWidth();	

	shared_ptr_type sp(new CBeamCounter(spanLen, supportHalfWidth, barDia));
	return sp;
}

inline unsigned GwaArx::Beam::_beam_length_check::CBeamCounter::SpanLength( void ) const
{
	return m_SpanLength;
}

unsigned GwaArx::Beam::_beam_length_check::CBeamCounter::SupportHalfWidth( void ) const
{
	return m_SupportHalfWidth;
}

unsigned GwaArx::Beam::_beam_length_check::CBeamCounter::BarDiameter( void ) const
{
	return m_BarDia;
}

void GwaArx::Beam::_beam_length_check::cmdBeamLenChck( void )
{
		
	BOOST_AUTO(pBC, CBeamCounter::acedNew()); 

	acutPrintf(TEXT("\n[span] = %-10d  [h.w.] = %-10d  [b.d.] = %d"),
		pBC->SpanLength(), pBC->SupportHalfWidth(), pBC->BarDiameter());
	acutPrintf(TEXT("\n[0.25] = %-10d  [0.15] = %-10d  [0.08] = %-10d  [0.15] = %d"),
		pBC->Top0_25(), pBC->Top0_15(), pBC->Bottom0_08(), pBC->Bottom0_15());
}


