#include "StdAfx.h"

#include "Beam_lapping_adjust.h"
#include "Util.h"
#include "Text.h"
#include "Configurable.h"

namespace
{
	enum sel_type { invalidSelType, onlyLappingLine, onlyDim, both };

	typedef	GwaArx::TextPatterns::CPatMainBar::shared_ptr_type rc_sp;
	typedef boost::shared_ptr<AcDbRotatedDimension> dim_sp;
	typedef boost::shared_ptr<AcDbLine> bar_sp;
	typedef std::vector<bar_sp> bar_sp_vec;

	bool _ssSentry( ads_name ss, sel_type & selType, dim_sp & spDim, bar_sp_vec & vecBars )
	{
		using namespace GwaArx::Util;

		long ssLen = 0;
		ret_RTNORM(acedSSLength(ss, &ssLen));

		if (ssLen != 1 && ssLen != 5)
		{
			acutPrintf(TEXT("\n*选取了过多或者过少的对象，应该是 1 个标注对象 ")
				TEXT("/ 1 个钢筋线 / 4 个钢筋线 + 1 个标注对象*"));
			return false;
		}

		ads_name ssLineOrDim;		
		std::vector<dim_sp> vecDims;
		bar_sp spABar;
		dim_sp spADim;		

		for(long n = 0; n != ssLen; ++n)
		{
			ret_RTNORM(acedSSName(ss, n, ssLineOrDim));
			if (spABar = name2SharedPtr<AcDbLine>(ssLineOrDim))
			{
				vecBars.push_back(spABar);
			}						
			else if (spADim = name2SharedPtr<AcDbRotatedDimension>(ssLineOrDim))
			{
				vecDims.push_back(spADim);
			}
			else
			{				
				xssert(!"ssgot an object of an invalid type, check the ssget filter");
				throw std::runtime_error("_ssSentry() - ssgot an object of an invalid type");
			}
		}
		
		if (vecBars.empty() && (1 == vecDims.size()))
		{
			spDim = vecDims[0];
			selType = onlyDim;
		}
		else if(vecDims.empty() && (1 == vecBars.size()))
		{
			selType = onlyLappingLine;
		}
		else if ((1 == vecDims.size()) && (4 == vecBars.size()))
		{
			spDim = vecDims[0];
			selType = both;
		}
		else
		{
			acutPrintf(TEXT("*无效的被选对象组合*"));
			return false;
		}	
		
		return true;
	}

	sel_type _getInput( rc_sp & spRC, dim_sp & spDim, bar_sp_vec & vecBars )
	{			
		using namespace boost;
		using namespace GwaArx::Util;
		using namespace GwaArx::TextPatterns;

		// cross window select the target(s).
		resbuf *ssFilter = acutBuildList(
			-4, TEXT("<OR"),
				// for line
				-4, TEXT("<AND"),
					RTDXF0, 	TEXT("LINE"), 
					8,			TEXT("MAINBAR"),
				-4, TEXT("AND>"),
				// for dimension
				-4, TEXT("<AND"),
					RTDXF0, 	TEXT("DIMENSION"),
					8,			TEXT("DIM-TEXT"),
				-4, TEXT("AND>"),
			-4, TEXT("OR>"),			
			RTNONE);
		xssert(ssFilter);

		sel_type selType = invalidSelType;
		GwaArx::Util::CAcEdSSGet ssget;
		ssget
			.setPrompt(TEXT("\n请选择 [要调整长度的那一条钢筋 / 要调整长度的那一个标注对象 / ")
			TEXT("四条钢筋 + 标注对象]："))
			.setFilter(ssFilter)
			.setSentry(boost::bind(_ssSentry, _1, ref(selType), ref(spDim), ref(vecBars)));
		ssget();	

		// pick the reference RC text.
		spRC = acedPickA<CPatMainBar>(TEXT("\n请选择配筋文字："));
		xssert(spRC);
		
		return selType;
	}

	void _caseOnlyDim( rc_sp spRC, dim_sp spDim )
	{
		#pragma message ("unimplemented!")
	}
	
	void _caseOnly4Bars( rc_sp rcRC, bar_sp_vec vecBars )
	{
		#pragma message ("unimplemented!")
	}
	
	void _caseBoth( rc_sp spRC, dim_sp spDim, bar_sp_vec vecBars )
	{
		#pragma message ("unimplemented!")
	}

}


void GwaArx::Beam::_lapping_adjust::cmdLappingAdjust( void )
{
	// prompt user to select objects
	rc_sp spRC;
	dim_sp spDim;
	bar_sp_vec vecBars;

	switch(_getInput(spRC, spDim, vecBars))
	{
	case onlyLappingLine:
		_caseOnly4Bars(spRC, vecBars);
		break;
	case onlyDim:
		_caseOnlyDim(spRC, spDim);
		break;
	case both:
		_caseBoth(spRC, spDim, vecBars);
		break;
	default:
		throw std::runtime_error("cmdLappingAdjust() - got a selection"
			"combination, which is not yet implemented");
	}
}