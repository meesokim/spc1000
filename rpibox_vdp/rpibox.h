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

#include <circle/multicore.h>
#include <circle/logger.h>
#include "screen8.h"
#include <SDCard/emmc.h>
#include <fatfs/ff.h>
#include <circle/memory.h>
#include <circle/types.h>
#include "tms9918.h"

class CRpiBox
#ifdef ARM_ALLOW_MULTI_CORE
	: public CMultiCoreSupport
#endif
{
public:
	CRpiBox (CScreenDevice8 *pScreen, CLogger *pLogger, CEMMCDevice *pEmmc, CMemorySystem *pMemorySystem);
	~CRpiBox (void);

#ifdef ARM_ALLOW_MULTI_CORE
	boolean Initialize (tms9918 vdp)	
	{ 
		this->vdp = vdp;
		return CMultiCoreSupport::Initialize(); 
	}
#endif
	void Run (unsigned nCore);
private:
	CScreenDevice8 *m_pScreen;
	CEMMCDevice		*m_pEmmc;
	CLogger *m_pLogger;
	FATFS			m_FileSystem;
	char files[256*256];
	char files2[256*256];
	char drive[256];
	char pattern[256];
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
	tms9918 vdp;
	void VdpBox();
#endif
	// flash the Act LED 10 times and click on audio (3.5mm headphone jack)
	char *fdd[3];
	CString FileName;
	FRESULT Result;
	FIL File;
	unsigned int nBytesRead;
	char diskbuf[258*256*8];
	char binbuf[256*256];
	unsigned char buffer[256*256*32];
	char tapbuf[256*256*32];
	int datain, dataout, data3, data0, p, q, t, cflag, rpi_idx, oldnum, readsize;
	unsigned char params[10];
	char *tmpbuf, *rpibuf;
};

#endif
