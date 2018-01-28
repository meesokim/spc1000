//
// mandelbrot.cpp
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
#include "rpibox.h"

#include <circle/types.h>
#include <circle/stdarg.h>
#include <string.h> 
#include <circle/memio.h>
#include <circle/bcm2835.h>
#include <circle/gpiopin.h>
#include <circle/util.h>
#include <assert.h>

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
//#define writeData(x) {write32 (ARM_GPIO_GPCLR0, 0xff); write32 (ARM_GPIO_GPSET0, x);}

inline void writeData(int x)
{
	write32 (ARM_GPIO_GPCLR0, 0xff); 
	write32 (ARM_GPIO_GPSET0, x);
}

CRpiBox::CRpiBox (CScreenDevice8 *pScreen, CLogger *pLogger, CEMMCDevice *pEmmc, CMemorySystem *pMemorySystem)
:	
#ifdef ARM_ALLOW_MULTI_CORE
	CMultiCoreSupport (pMemorySystem),
#endif
	m_pScreen (pScreen),
	m_pEmmc (pEmmc),
	m_pLogger (pLogger)
{
	p = 0;
	q = 0;
	t = 0;
	data0 = 0;
	dataout = 0;
	data3 = 0;
	datain = 0;
	nBytesRead = 0;
	tmpbuf = 0;
	rpi_idx = 0;
	oldnum = 0;
	readsize = 0;
//	memset(buffer, 0, 256*256);
}

CRpiBox::~CRpiBox (void)
{
	m_pScreen = 0;
}

void CRpiBox::Run (unsigned nCore)
{
	switch (nCore)
	{
	case 0:
		printf("RpiBox Started (Run)\n");
		Storage();
		break;
#ifdef ARM_ALLOW_MULTI_CORE
	case 1:
		VdpBox();
		break;
#endif
	}
}

char *CRpiBox::strrchr(const char *s, int ch)
{
    char *start = (char *) s;

    while (*s++);
    while (--s != start && *s != (char) ch);
    if (*s == (char) ch) return (char *) s;

    return 0;
}

char * CRpiBox::fnRPI_FILES(char *drive, char *pattern)
{
	// Show contents of root directory
	DIR Directory;
	FILINFO FileInfo;
	CString FileName;
	FRESULT Result = f_findfirst (&Directory, &FileInfo,  DRIVE "/", "*.cas");
	int len = 0, len2= 0, length = 0;
	memset(files, 0, 256*256);
	memset(files2, 0, 256*256);
	for (unsigned i = 0; Result == FR_OK && FileInfo.fname[0]; i++)
	{
		if (!(FileInfo.fattrib & (AM_HID | AM_SYS)))
		{
			strcpy(files+len, FileInfo.fname);
			strcpy(files2+len2, FileInfo.fname);
			length = strlen(FileInfo.fname);
			len += length;
			len2 += length > 24 ? 24 : length;
			*(files+(len++))='\\';
			*(files2+(len2++))='\\';
		}

		Result = f_findnext (&Directory, &FileInfo);
	}
	printf ("%s\n", files);
	return files;
}

void CRpiBox::Storage()
{
	int a, addr, wr;
	printf("RpiBox Started (Storage)\n");
	a = 0;
	addr = 0;
	wr = 0;
	// Mount file system
	if (f_mount (&m_FileSystem, DRIVE, 1) != FR_OK)
	{
		error ("Cannot mount drive: %s", DRIVE);
	} 
	printf("FileSystem mounted (Storage)\n");
	data0 = 0xff;
	printf("completed\n");
	write32 (ARM_GPIO_GPFSEL0, 0x249249);
	write32 (ARM_GPIO_GPFSEL0+4, 0);
	write32 (ARM_GPIO_GPFSEL0+8, 0);
	while(1)
	{
		if (!(GPIO & RPSPC_EXT))
		{
			a = GPIO;
			addr = (a & ADDR) >> RPSPC_A0_PIN;
			wr = (a & RPSPC_RD) != 0;
#ifdef ARM_ALLOW_MULTI_CORE
			if (a & RPSPC_A11)
			{
				if (wr)
				{
					if (addr & 1)
						tms9918_writeport1(vdp, a & 0xff);
					else
						tms9918_writeport0(vdp, a & 0xff);
					//m_pScreen->Write('W');
				}
				else
				{
					GPIO_CLR(0xff);
					if (addr & 1)
						GPIO_SET(tms9918_readport1(vdp));
					else
						GPIO_SET(tms9918_readport0(vdp));
					//m_pScreen->Write('R');
				}
			}
			else
#endif				
			{
				switch (addr)
				{
					case 0:
						if (wr) 
							datain = a & 0xff;
						else
							writeData(data0);
						break;
					case 1:
						if (!wr)
						{
							writeData(dataout);
						}
						break;
					case 2:
						if (wr)
						{
							readCommand(a & 0xf0);
						}
						else
						{
							writeData(cflag);
						}
						break;
					case 3:
						if (wr)
						{
							data3 = tapbuf[t++] == '1' ? 1 : 0;	
						}
						else
						{
							writeData(data3);
						}
					default:
						break;
				}
			}
			while(!(GPIO & RPSPC_EXT));
		}
	}
}

void CRpiBox::readCommand(int cmd)
{
	int blocks, drv, tracks, sectors;
	int len, len0, fileno;
	char filename[256] = {0};
	char *point;
	switch (cmd & 0xf0)
	{
		case rATN: // 0x80 --> 0x02
			cflag = wRFD; 
			p = 0;
			break;
		case rDAV: // 0x10 --> 0x04
			if (!(cflag & wDAC))
			{
				cflag |= wDAC;
				if (p < 10)
					params[p] = datain;
				if (p == 0)
				{
					printf ("[%d]", datain);
				}
				q = 0;
				switch (params[0])
				{
					case SDINIT:
						buffer[0] = 100;
						printf("SDINIT\n");
						Result = f_open (&File, DRIVE FILENAME, FA_READ | FA_OPEN_EXISTING);
						if (Result != FR_OK)
						{
							printf ("loading failed: %s\n", FILENAME);
						}
						else
						{
							f_read(&File, binbuf, sizeof binbuf, &nBytesRead);
//													f_close (&File);
							printf ("loading file: %s(%d)\n", FILENAME, nBytesRead);
							fdd[0] = binbuf;
						}
						break;
					case SDWRITE:		
						if (p == 4)
						{
							blocks = params[1];
							drv = params[2];
							tracks = params[3];
							sectors = params[4];
							tmpbuf = fdd[drv] + (tracks * 16 + (sectors - 1))*256;
						} else if (p > 4)
						{
							tmpbuf[p - 5] = datain;
						}
						break;
					case SDREAD:
						if (p == 4) 
						{
							blocks = params[1];
							drv = params[2];
							tracks = params[3];
							sectors = params[4];												
							readsize = 256 * blocks;
							memcpy(diskbuf, fdd[drv]+(tracks * 16 + (sectors - 1))*256, readsize);
							printf ("SDREAD(%d) %d block(s), drive %d, track %d, sector %d, %dbytes)\n", p, blocks, drv, tracks, sectors, readsize);
						}
						break;
					case SDSEND:
						memcpy(buffer, diskbuf, readsize);
						printf ("SDSEND(%d) %02x %02x %02x %02x (%d)\n", p, buffer[0],  buffer[1], buffer[2], buffer[3], readsize);
						break;
					case SDCOPY:
						memcpy(fdd[params[5]]+(params[6] * 16 + (params[7]-1))*256, fdd[params[2]]+(params[3]*16+(params[4]-1))*256, 256 * params[1]);
						break;
					case SDSTATUS:
						buffer[0] = 0xc0;
						printf("SDSTATUS\n");
						break;
					case SDDRVSTS:
						buffer[0] = 0xff;
						printf("SDDRVSTS\n",q);
						break;
					case RPI_FILES:
						if (p == 0)
						{
							rpi_idx = 0;
							strcpy(drive, "SD:/");
							strcpy(pattern, "*.tap");
							rpibuf = drive;
						} 
						if (datain == 0)
						{
							if (rpibuf == pattern || p == 0)
							{
								tmpbuf = fnRPI_FILES(drive, pattern);
								strcpy((char*)buffer, files2);
								printf ("RPI_FILES: drive=%s, pattern=%s\n%s\n", drive, pattern, files2);
							}
						}
						else if (params[p] == '\\')
						{
							rpibuf[rpi_idx] = 0;
							rpi_idx = 0;
							rpibuf = pattern;
						}
						else
							rpibuf[rpi_idx++] = datain;
						break;
						case RPI_LOAD:
						if (p == 2)
						{
							oldnum = fileno = params[1] + params[2] * 256;
							printf ("fileno:%d\n", fileno);
							len0 = 0;
							len = 0;
							while(len0 < fileno)
							{
								if (files[len++] == '\\')
									len0++;
							}	
							len0 = 0;
							while(files[len+len0++] != '\\'); 
							strcpy(filename,  DRIVE);
							memcpy(filename+4, files+len, len0);
							*(filename+4+len0-1)=0;
							FILINFO fno;
							f_stat(filename, &fno);
							printf ("RPI_LOAD: No.%d %s (%d)\n", fileno, filename, fno.fsize);
							Result = f_open (&File, filename, FA_READ | FA_OPEN_EXISTING);
							if (Result != FR_OK)
							{
								printf("loading failed: %s\n", filename);
							}
							else
							{
								f_read(&File, buffer, fno.fsize, &nBytesRead);
								f_close(&File);
								point = filename;
								while(*point == '.' || *point == 0) point++;
								if(strcmp(point,".cas") == 0) 
								{
									for(unsigned i = 15; i < nBytesRead; i++)
									{
										for (unsigned j = 0; j < 8; j++)
											tapbuf[i*8+j]   = ((buffer[i] >> (7-j)) & 1) + '0';
									}
								}
								else
									memcpy(tapbuf, buffer, nBytesRead);
								printf ("loading successful: %s\n", filename);
								t = 0;
							}
						}
						break;
					case RPI_OLDNUM:
						buffer[0] = oldnum & 0xff;
						buffer[1] = oldnum >> 8;	
//											printf("oldnum = %d\n", oldnum);
						break;								
					default:
						buffer[0] = cmd * cmd;
						break;
				}
			}									
			break;
		case rRFD: // 0x20 --> 0x01
			cflag |= wDAV;
			break;
		case rDAC: // 0x40 --> 0x00
			if (cflag & wDAV)
			{
				cflag &= ~wDAV;
				q++;
			}
			break;
		case 0: // 0x00 --> 0x00
			if (cflag & wDAC)
			{
				cflag &= ~wDAC;
				p++;
			}
			else if (cflag & wDAV)
			{
				dataout = buffer[q];
			}
			break;
		default:
			break;
	}	
}

#ifdef ARM_ALLOW_MULTI_CORE
void CRpiBox::VdpBox()
{
	int time = 0;
	while(true)
	{
		if (time++ > (int)(250000000/30/261))
		{
			tms9918_periodic(vdp);
			time = 0;
		};
	}	
}
#endif

int CRpiBox::printf(const char* format, ...)
{
	CString str;
	va_list argptr;
    va_start(argptr, format);
    str.FormatV(format, argptr);
    va_end(argptr);
	m_pScreen->Write(str, str.GetLength());	
	return 0;
}

