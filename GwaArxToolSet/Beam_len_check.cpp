#include "StdAfx.h"

#include "Util.h"
#include "Configurable.h"
#include "Text.h"
#include "Beam.h"

unsigned GwaArx::Beam::_beam_length_check::CBeamCounter::retTop0_25( void ) const
{
	unsigned ret = (std::ceil((m_SpanLength * 0.25 + m_SupportHalfWidth) / 100)) * 100;
	unsigned _46d = GwaArx::Configurations::CGwaDataSheet::LappingLength_46D(m_BarDia);
	ret = (ret > _46d) ? ret : _46d;
	return ret;
}

unsigned GwaArx::Beam::_beam_length_check::CBeamCounter::retTop0_15( void ) const
{
	unsigned ret = (std::ceil((m_SpanLength * 0.15 + m_SupportHalfWidth) / 100)) * 100;
	unsigned _46d = GwaArx::Configurations::CGwaDataSheet::LappingLength_46D(m_BarDia);
	ret = (ret > _46d) ? ret : _46d;
	return ret;
}

unsigned GwaArx::Beam::_beam_length_check::CBeamCounter::retBottom0_08( void ) const
{
	// round off to multiple of 100
	return (int(m_SpanLength * 0.08) / 100) * 100;
}

unsigned GwaArx::Beam::_beam_length_check::CBeamCounter::retBottom0_15( void ) const
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
			xssert(!"should not got here!");
			throw std::runtime_error("should not got here!");
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

inline unsigned GwaArx::Beam::_beam_length_check::CBeamCounter::spanLength( void ) const
{
	return m_SpanLength;
}

unsigned GwaArx::Beam::_beam_length_check::CBeamCounter::supportHalfWidth( void ) const
{
	return m_SupportHalfWidth;
}

unsigned GwaArx::Beam::_beam_length_check::CBeamCounter::barDiameter( void ) const
{
	return m_BarDia;
}

void GwaArx::Beam::_beam_length_check::cmdBeamLenChck( void )
{
	// the constructor of CBeamCounter will the dirty job, such as 
	// asking for input, calculating the results.
	
	BOOST_AUTO(pBC, CBeamCounter::acedNew()); 

	acutPrintf(TEXT("\n[梁跨] = %-10d  [半制作] = %-10d  [钢筋直径] = %d"),
		pBC->spanLength(), pBC->supportHalfWidth(), pBC->barDiameter());
	acutPrintf(TEXT("\n[上 0.25] = %-10d  [上 0.15] = %-10d\n[下 0.15] = %-10d  [下 0.08] = %d"),
		pBC->retTop0_25(), pBC->retTop0_15(), pBC->retBottom0_08(), pBC->retBottom0_15());
}


