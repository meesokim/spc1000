#ifndef _kernel_h
#define _kernel_h

#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/screen.h>
#include <circle/types.h>
#include <circle/usb/usbhcidevice.h>
#include <circle/usb/usbkeyboard.h>
#include <circle/sched/scheduler.h>
#include <circle/sound/soundbasedevice.h>
#include <vc4/vchiq/vchiqdevice.h>
#include <vc4/sound/vchiqsoundbasedevice.h>

#include <SDCard/emmc.h>
#include <fatfs/ff.h>

#define __circle__
#include "cassette.h"

extern "C" {
#include "emu2149.h"
}

enum TShutdownMode
{
	ShutdownNone,
	ShutdownHalt,
	ShutdownReboot
};

// VCHIQ sound device that generates PSG samples directly via GetChunk
class CSPCSoundDevice : public CVCHIQSoundBaseDevice
{
public:
	CSPCSoundDevice (CVCHIQDevice *pVCHIQ, PSG **ppPSG, unsigned nRate)
	:	CVCHIQSoundBaseDevice (pVCHIQ, nRate, 4000, VCHIQSoundDestinationAuto),
		m_ppPSG (ppPSG) {}

	unsigned GetChunk (s16 *pBuffer, unsigned nChunkSize) override;

private:
	PSG **m_ppPSG;
};

class CKernel
{
public:
	CKernel (void);
	~CKernel (void);

	boolean Initialize (void);
	TShutdownMode Run (void);

	static void KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6]);

private:
	CMemorySystem		m_Memory;
	CKernelOptions		m_Options;
	CDeviceNameService	m_DeviceNameService;
	CExceptionHandler	m_ExceptionHandler;
	CInterruptSystem	m_Interrupt;
	CTimer			m_Timer;
	CLogger			m_Logger;
	CActLED			m_ActLED;
	CScreenDevice		m_Screen;
	CVCHIQDevice		m_VCHIQ;
	CUSBHCIDevice		m_USBHCI;
	CScheduler		m_Scheduler;
	CUSBKeyboardDevice	*m_pKeyboard;
	CEMMCDevice		m_EMMC;
	FATFS			m_FileSystem;

public:
	Cassette		m_Cassette;
	CSPCSoundDevice		*m_pSound;
	PSG			*m_pPSG;
};

#endif
