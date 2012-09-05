#include "StdAfx.h"

#include "Util.h"
#include "Configurable.h"

GwaArx::Util::_aced_get::CAcEdSSGetBase::~CAcEdSSGetBase()
{
	if (m_rb)
	{
		acutRelRb(m_rb);
	}
}