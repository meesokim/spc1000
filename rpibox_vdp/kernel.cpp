//
// kernel.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2015  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "kernel.h"

static const char FromKernel[] = "kernel";

CKernel::CKernel (void)
:
#ifdef ARM_ALLOW_MULTI_CORE
	m_Screen(320,240),
#else	
	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
#endif
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED),
	m_Rpibox (&m_Screen, &m_Logger, &m_EMMC, &m_Memory)
{
	m_ActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;
		
	if (bOK)
	{
		bOK = m_Screen.Initialize ();
	}
	if (bOK)
	{
		bOK = m_Logger.Initialize (&m_Screen);
	}
	if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}
	if (bOK)
	{
		bOK = m_EMMC.Initialize ();
	}
	if (bOK)
	{
		bOK = m_Timer.Initialize ();
	} 
#ifdef ARM_ALLOW_MULTI_CORE	
	if (bOK)
	{
		bOK = m_Rpibox.Initialize(tms9918_create());
	}
#endif	
	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);
	m_Logger.Write (FromKernel, LogNotice, "SPC-1000 Extension");	
	printf("RpiBox Started (Kernel)\n");
	m_Rpibox.Run (0);
	return ShutdownHalt;
}