//
// rpibox.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2015  R. Stange <rsta2@o2online.de>
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
#ifndef _rpibox_h
#define _rpibox_h

#define WIDTH 640
#define HEIGHT 480

#include <circle/multicore.h>
#include <circle/logger.h>
#include "screen8.h"
#include <SDCard/emmc.h>
#include <fatfs/ff.h>
#include <circle/memory.h>
#include <circle/types.h>
#include <circle/util.h>
#include <circle/bcm2835.h>
#include <circle/gpiopin.h>
#include <circle/memio.h>
#include "tms9918.h"

#define RPSPC_D0	(1 << 0)
#define MD00_PIN	0
#define RPSPC_WR	(1 << 14)
#define RPSPC_RD	(1 << 15)
#define RPSPC_EXT	(1 << 17)
#define RPSPC_RST   (1 << 27)
#define RPSPC_CLK	(1 << 18)

#define RPSPC_A0	(1 << 22)
#define RPSPC_A1	(1 << 23)
#define RPSPC_A11	(1 << 24)
#define RPSPC_A0_PIN 22

#define ADDR  (RPSPC_A0 | RPSPC_A1)
#define ADDR0 0
#define ADDR1 RPSPC_A0
#define ADDR2 RPSPC_A1

#define SDINIT		0
#define SDWRITE		1
#define SDREAD		2
#define SDSEND  	3
#define SDCOPY		4
#define SDFORMAT	5
#define SDSTATUS	6
#define SDDRVSTS	7
#define SDRAMTST	8
#define SDTRANS2	9
#define SDNOACT		10
#define SDTRANS1	11
#define SDRCVE		12
#define SDGO		13
#define SDLOAD		14
#define SDSAVE		15
#define SDLDNGO		16
#define RPI_FILES	0x20
#define RPI_LOAD	0x21
#define RPI_OLDNUM	0x23

#define READY 			0
#define READ_FOR_DATA  	1
#define DATA_VALID 		2
#define RECEIVED		3
#define rATN 0x80
#define rDAC 0x40
#define rRFD 0x20
#define rDAV 0x10
#define wDAC 0x04
#define wRFD 0x02
#define wDAV 0x01

#define DRIVE		"SD:"
#define FILENAME	"/spc1000.bin" 

#define GPIO (read32 (ARM_GPIO_GPLEV0))
#define GPIO_CLR(x) write32 (ARM_GPIO_GPCLR0, x)
#define GPIO_SET(x) write32 (ARM_GPIO_GPSET0, x)

class CRpiBox
#ifdef ARM_ALLOW_MULTI_CORE
	: public CMultiCoreSupport
#endif
{
public:
	CRpiBox (CScreenDevice8 *pScreen, CLogger *pLogger, CEMMCDevice *pEmmc, CMemorySystem *pMemorySystem);
//	CRpiBox (CMemorySystem *pMemorySystem) : CMultiCoreSupport(pMemorySystem) {}
	~CRpiBox (void);

#ifndef ARM_ALLOW_MULTI_CORE
	boolean Initialize (void)	{ return TRUE; }
#endif

	void Run (unsigned nCore);
private:
	CScreenDevice8 	*m_pScreen;
	CEMMCDevice		*m_pEmmc;
	CLogger 		*m_pLogger;
	FATFS			m_FileSystem;
	char *fnRPI_FILES(char *drive, char *pattern);
	char *strrchr(const char *s, int ch);
//	void *memcpy2(void *pDest, const void *pSrc, size_t nLength);
	void readCommand(int);
	int printf(const char *format, ...);
	void error(const char *format, ...)
	{
		CString str;
		va_list argptr;
		va_start(argptr, format);
		str.FormatV(format, argptr);
		va_end(argptr);
		m_pLogger->Write ("RpiBox", LogPanic, str);	
	}
	void Storage();
#ifdef ARM_ALLOW_MULTI_CORE
	void VdpBox();
#endif
};

#endif
