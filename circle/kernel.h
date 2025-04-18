//
// kernel.h
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
#ifndef _kernel_h
#define _kernel_h

#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/types.h>
#include <circle/i2cmaster.h>
#include <circle/usb/usbcontroller.h>
#include <circle/usb/usbhcidevice.h>
#include <circle/usb/usbkeyboard.h>
#include <circle/string.h>
#include <circle/util.h>

#ifdef USE_VCHIQ_SOUND
	#include <vc4/vchiq/vchiqdevice.h>
#endif

#include "spc1000.h"

enum TShutdownMode
{
	ShutdownNone,
	ShutdownHalt,
	ShutdownReboot
};

class CKernel
{
public:
	CKernel (void);
	~CKernel (void);

	boolean Initialize (void);

	TShutdownMode Run (void);
	static unsigned int GetTicks() 
	{
		if (s_pThis) {
			// return s_pThis->m_Timer.GetClockTicks () / 1000;
		}
		return 0;
	}
private:
	// do not change this order
	CActLED				m_ActLED;
	CKernelOptions		m_Options;
	CDeviceNameService	m_DeviceNameService;
	CScreenDevice		m_Screen;
	CExceptionHandler	m_ExceptionHandler;
	CInterruptSystem	m_Interrupt;
	CTimer				m_Timer;
	CLogger				m_Logger;
	// CI2CMaster			m_I2CMaster;
	// CUSBController		*m_pUSB;
	// CUSBHCIDevice		m_USBHCI;
	// CEMMCDevice			m_EMMC;
	// CKeyboard			m_Keyboard;
	// CUSBKeyboardDevice *m_pKeyboard;
	// FATFS				m_FileSystem;
	// CMiniOrgan		*m_pMiniOrgan;
	static void KeyboardRemovedHandler (CDevice *pDevice, void *pContext);
	static void KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6]);
	static CKernel *s_pThis;	
};

#endif
