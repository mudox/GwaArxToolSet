#include "StdAfx.h"

#include "Beam_lapping_adjust.h"
#include "Util.h"
#include "Text.h"
#include "Configurable.h"

namespace
{
	enum sel_type { onlyDropLine, only4Bars, onlyDim, both };
	typedef	GwaArx::TextPatterns::CPatMainBar::shared_ptr_type rc_sp;
	typedef boost::shared_ptr<AcDbRotatedDimension> dim_sp;
	typedef boost::shared_ptr<AcDbLine> bar_sp;

	sel_type _getInput( rc_sp & spRC, dim_sp & spDim, bar_sp * arr4Bars )
	{			
		#error unimplemented!
		return onlyDim;
	}

	void _caseOnlyDim( rc_sp spRC, dim_sp spDim )
	{
		#pragma message ("unimplemented!")
	}
	
	void _caseOnly4Bars( rc_sp rcRC, bar_sp * arr4Bars )
	{
		#pragma message ("unimplemented!")
	}
	
	void _caseBoth( rc_sp spRC, dim_sp spDim, bar_sp * arr4Bars )
	{
		#pragma message ("unimplemented!")
	}

}


void GwaArx::Beam::_lapping_adjust::cmdLappingAdjust( void )
{
	// prompt user to select objects
	rc_sp spRC;
	dim_sp spDim;
	bar_sp arr4Bars[4];

	switch(_getInput(spRC, spDim, arr4Bars))
	{
	case only4Bars:
		_caseOnly4Bars(spRC, arr4Bars);
		break;
	case onlyDim:
		_caseOnlyDim(spRC, spDim);
		break;
	case both:
		_caseBoth(spRC, spDim, arr4Bars);
		break;
	default:
		throw std::runtime_error("cmdLappingAdjust() - got a selection"
			"combination, which is not yet implemented");
	}
}