//
// kernel.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2023  R. Stange <rsta2@o2online.de>
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
#include <circle/usb/usbhcidevice.h>
#include <circle/usb/gadget/usbmidigadget.h>
#include <circle/machineinfo.h>
#include <assert.h>

//#define USB_GADGET_MODE

#define DRIVE		"SD:"

static const char FromKernel[] = "kernel";

CKernel::CKernel (void)
:	m_Screen (640, 480),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_I2CMaster (CMachineInfo::Get ()->GetDevice (DeviceI2CMaster), TRUE),
#ifndef USB_GADGET_MODE
	m_pUSB (new CUSBHCIDevice (&m_Interrupt, &m_Timer, TRUE)), // TRUE: enable plug-and-play
#else
	m_pUSB (new CUSBMIDIGadget (&m_Interrupt)),
#endif
	m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED),
	m_USBHCI (&m_Interrupt, &m_Timer, TRUE),		// TRUE: enable plug-and-play
	m_Keyboard()
	// m_pMiniOrgan (0)
{
	m_ActLED.Blink (5);	// show we are alive
	m_pKeyboard = nullptr;
}

CKeyboard *CKeyboard::s_pThis = 0;
CKernel *CKernel::s_pThis = 0;

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;
	// if (bOK) { bOK = m_DeviceNameService.Initialize (); }
	if (bOK) { bOK = m_Screen.Initialize (); }
	if (bOK) { bOK = m_Logger.Initialize (&m_Screen); }
	if (bOK) { bOK = m_Interrupt.Initialize (); }
	if (bOK) { bOK = m_Timer.Initialize (); }
	if (bOK) { bOK = m_I2CMaster.Initialize (); }
	if (bOK) { assert (m_pUSB); bOK = m_pUSB->Initialize (); }
	if (bOK) { bOK = m_EMMC.Initialize (); }
	if (bOK) { bOK = m_USBHCI.Initialize (); }
	// if (bOK) { m_pMiniOrgan = new CMiniOrgan (&m_Interrupt, &m_I2CMaster); bOK = m_pMiniOrgan->Initialize (); }
	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);
	m_Logger.Write (FromKernel, LogNotice, "SPC-1000");
	if (f_mount (&m_FileSystem, DRIVE, 1) != FR_OK)
	{
		m_Logger.Write (FromKernel, LogPanic, "Cannot mount drive: %s", DRIVE);
	}
	while(true)
	{
		boolean bUpdated = m_USBHCI.UpdatePlugAndPlay ();
		if (  bUpdated && m_pKeyboard == nullptr)
		{
			m_pKeyboard = (CUSBKeyboardDevice *) m_DeviceNameService.GetDevice ("ukbd1", FALSE);
			if (m_pKeyboard != nullptr)
			{
				m_pKeyboard->RegisterRemovedHandler (KeyboardRemovedHandler);

#if 0	// set to 0 to test raw mode
				m_pKeyboard->RegisterShutdownHandler (ShutdownHandler);
				m_pKeyboard->RegisterKeyPressedHandler (KeyPressedHandler);
#else
				m_pKeyboard->RegisterKeyStatusHandlerRaw (m_Keyboard.KeyStatusHandlerRaw);
#endif
				m_Logger.Write (FromKernel, LogNotice, "Just type something!");			
			}
		}
	}
	for (unsigned nCount = 0; true; nCount++)
	{
		// This must be called from TASK_LEVEL to update the tree of connected USB devices.
		assert (m_pUSB);
		// boolean bUpdated = m_pUSB->UpdatePlugAndPlay ();
		// m_pMiniOrgan->Process (bUpdated);
		m_Screen.Rotor (0, nCount);
	}
	return ShutdownHalt;
}

void CKernel::KeyboardRemovedHandler (CDevice *pDevice, void *pContext)
{
	assert (s_pThis != nullptr);
	CLogger::Get ()->Write (FromKernel, LogDebug, "Keyboard removed");

	s_pThis->m_pKeyboard = 0;
}
