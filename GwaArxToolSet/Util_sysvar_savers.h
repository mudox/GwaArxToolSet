#pragma once

#include "StdAfx.h"

#define  SUPRESS_ORTHO_SNAP_MODE_A_WHILE GwaArx::Util::FreeCursorAWhile \
_close_ortho_snap_mode_a_while_ (static_cast<GwaArx::Util::sysvar_bit>(\
	GwaArx::Util::setORTHOMODE | \
	GwaArx::Util::setSNAPMODE))

#define  SUPRESS_ORTHO_SNAP_OS_MODE_A_WHILE GwaArx::Util::FreeCursorAWhile \
_close_os_snap_ortho_mode_a_while_ (static_cast<GwaArx::Util::sysvar_bit>(\
	GwaArx::Util::setORTHOMODE | \
	GwaArx::Util::setSNAPMODE | \
	GwaArx::Util::setOSMODE))

namespace GwaArx
{
	namespace Util
	{
		namespace _sysvar_savers
		{
			enum sysvar_bit
			{	
				setNULL			= 0,
				setOSMODE		= 1 << 0,
				setSNAPMODE		= 1 << 1,
				setORTHOMODE	= 1 << 2,
				setALL			= setOSMODE | setSNAPMODE | setORTHOMODE
			};

			class CursorStatusAutoRecorver
			{
			public:
				typedef short int_type;

				enum // OSMODE bits
				{
					osNULL	= 0,		// 无 
					osEND 	= 1,		// END（端点） 
					osMID 	= 2,		// MID（中点） 
					osCEN 	= 4,		// CEN（圆心）
					osNOD 	= 8,		// NOD（节点）
					osQUA 	= 16,		// QUA（象限点）
					osINT	= 32,		// INT（交点）
					osINS 	= 64,		// INS（插入点）
					osPER 	= 128,		// PER（垂足）
					osTAN 	= 256,		// TAN（切点）
					osNEA 	= 512,		// NEA（最近点）
					osCLR 	= 1024,		// 清除所有对象捕捉
					osAPP 	= 2048,		// APP（外观交点）
					osEXT 	= 4096,		// EXT（延伸）
					osPAR	= 8192		// PAR（平行）
				};

			public:
				CursorStatusAutoRecorver( const sysvar_bit which );
				~CursorStatusAutoRecorver();

			private:
				sysvar_bit		m_which;
				int_type 	m_osMode;
				int_type 	m_snapMode;
				int_type 	m_orthoMode;

			};

			class FreeCursorAWhile : public CursorStatusAutoRecorver
			{
			public:
				FreeCursorAWhile( const sysvar_bit & which );
				~FreeCursorAWhile();
			};
		}
	}
}