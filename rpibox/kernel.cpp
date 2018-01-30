//
// kernel.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2016  R. Stange <rsta2@o2online.de>
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

#include <circle/types.h>
#include <stdint.h>
#include <stdio.h>
#include <circle/stdarg.h>
#include <string.h>

#include <circle/debug.h>
#include <circle/memio.h>
#include <circle/bcm2835.h>
#include <circle/gpiopin.h>
#include <circle/util.h>
#include <assert.h>

#define RPSPC_D0	(1 << 0)
#define MD00_PIN	0
#if 1
#define RPSPC_WR	(1 << 14)
#define RPSPC_RD	(1 << 15)
#define RPSPC_EXT	(1 << 17)
#define RPSPC_RST   (1 << 27)
#define RPSPC_CLK	(1 << 18)
#else
#define RPSPC_WR	(1 << 14)
#define RPSPC_RD	(1 << 15)
#define RPSPC_EXT	(1 << 18)
#define RPSPC_RST   (1 << 17)
#define RPSPC_CLK	(1 << 27)
#endif
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

#include "tms9918.h"
#include "video.h"
#include "lib.h"

extern "C" {
#include <circle/string.h>
extern void ExtraCoreSetup (void);
};

#include "kernel.h"

extern CKernel Kernel;

extern "C" {
int printf(const char* format, ...)
{
	CString str;
	va_list argptr;
    va_start(argptr, format);
    str.FormatV(format, argptr);
    va_end(argptr);
	Kernel.m_Screen.Write(str, strlen(str));
	return 0;
};	
};

static const char FromKernel[] = "kernel";
#define WIDTH 320
#define HEIGHT 240
int wgap = 0;
int hgap = 0;
tms9918 vdp;

extern int tms9918_palbase_red[];
extern int tms9918_palbase_green[];
extern int tms9918_palbase_blue[];

extern volatile uint32_t setStackPtr;
extern volatile uint32_t setIrqStackPtr;

#define CORE0_MBOX3_SET             0x4000008C

typedef int func(void);
typedef void (*fn)(void);

#ifdef ARM_ALLOW_MULTI_CORE

uint32_t get_core_id(void)
{
    uint32_t core_id;
    asm volatile ("mrc p15, 0, %0, c0, c0,  5" : "=r" (core_id));
    return core_id & 0x3;
}
 
static void core_enable(uint32_t core, uint32_t addr)
{
    // http://www.raspberrypi.org/forums/viewtopic.php?f=72&t=98904&start=25
//    volatile uint32_t *p;
//	setStackPtr = 0x4000;
//	setIrqStackPtr = 0x7000;  
    *(uint32_t*)(CORE0_MBOX3_SET + 0x10 * core) = addr;
	printf("core=%u\n", get_core_id());
//	while (setStackPtr != 0);
//	if (get_core_id()==core)
//	{
//		func* f = (func*)addr;
//		f();
//	}
}

#endif

void core1_main(void)
{
	int time = 0;
	asm ("mrc p15, 0, r0, c0, c0, 5");
	printf("core1_main\n");
	while(true)
	{
		time++;
		if (time > 250000000/30/261)
		{
			tms9918_periodic(vdp);
			Kernel.m_Screen.Write('V');
			time = 0;
		};
	}
}
	
void video_setsize(int x, int y)
{
	wgap = (WIDTH-x)/2;
	hgap = (HEIGHT-y)/2;
}
void video_setpal(int num_colors, int *red, int *green, int *blue)
{
	for(int i = 3; i < num_colors; i++)
	{
		Kernel.m_Screen.SetPalette(i, (u16)COLOR16(red[i], green[i], blue[i]));
	}
	Kernel.m_Screen.UpdatePalette();	
	Kernel.m_Screen.Write ("PALETTE\n", 8);	
}

unsigned char *video_get_vbp(int line)
{
	return Kernel.m_Screen.GetBuffer() + hgap * WIDTH + wgap;
}

void video_display_buffer()
{
}

CKernel::CKernel (void)
://	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Screen(320,240),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel ()),
	m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED)
#ifdef ARM_ALLOW_MULTI_CORE	
	,m_Core(&m_Memory)
#endif	

{
	m_ActLED.Blink (1);	// show we are alive
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
		vdp = tms9918_create();
	}
	
	if (bOK)
	{
		bOK = m_Logger.Initialize (&m_Screen);
	}

	if (bOK)
	{
		bOK = m_Timer.Initialize ();
	} 
	
	if (bOK)
	{
		bOK = m_EMMC.Initialize ();
	}
#ifdef ARM_ALLOW_MULTI_CORE	
	if (bOK)
	{
		bOK = m_Core.Initialize();
	}
#endif	
	return bOK;
}
#define GPIO (read32 (ARM_GPIO_GPLEV0))
#define GPIO_CLR(x) write32 (ARM_GPIO_GPCLR0, x)
#define GPIO_SET(x) write32 (ARM_GPIO_GPSET0, x)

char * CKernel::fnRPI_FILES(char *drive, char *pattern)
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
	FileName.Format ("%s\n", files);
	m_Screen.Write ((const char *) FileName, FileName.GetLength ());			
	return files;
}

char *strrchr(const char *s, int ch)
{
    char *start = (char *) s;

    while (*s++);
    while (--s != start && *s != (char) ch);
    if (*s == (char) ch) return (char *) s;

    return 0;
}

TShutdownMode CKernel::Run (void)
{
	// flash the Act LED 10 times and click on audio (3.5mm headphone jack)
	int a, addr, wr, datain, dataout, data3, readsize, data0;
	static char diskbuf[258*256*8], buffer[256*256*32], tapbuf[256*256*32];
	unsigned char cflag, blocks, drv, tracks, sectors;
	char *tmpbuf, *rpibuf;
	int params[10], p, q, t, rpi_idx, len, len0, fileno;
	char * fdd[3];
	//int f[256] = {0x11,0x7,0xcb,0xcd,0xf3,0x07,0xc9,0x48,0x65,0x6c,0x6c,0x20,0x77,0x6f,0x72,0x6c,0x64,0x21,0x00};	
	char f[256*256];
	char filename[256];
	static int oldnum = 0;
//	int args[] {0,4,4,0,7,1,1,0,4,4,0,4,4,2,6,6,0,0,0,0,0,2};
//	int x = 0;

	fdd[0] = f;
	rpibuf = 0;
	rpi_idx = 0;
	datain = 0;
	dataout = 0;
	blocks = cflag = p = q = t = data0 = data3 = 0;
	readsize = 0;
	tmpbuf = 0;
	CSpinLock spinlock;
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);
	m_Logger.Write (FromKernel, LogNotice, "SPC-1000 Extension");	
	
	// Mount file system
	if (f_mount (&m_FileSystem, DRIVE, 1) != FR_OK)
	{
		m_Logger.Write (FromKernel, LogPanic, "Cannot mount drive: %s", DRIVE);
	} 
	
	// Create file and write to it
	CString FileName;
	FRESULT Result;
#if 1
	FIL File;
	Result= f_open (&File, DRIVE FILENAME, FA_READ | FA_OPEN_EXISTING);
	if (Result != FR_OK)
	{
		m_Logger.Write (FromKernel, LogPanic, "Cannot open file: %s", FILENAME);
	}
	else
	{
		FileName.Format ("loading file: %s\n", FILENAME);
		m_Screen.Write ((const char *) FileName, FileName.GetLength ());
	}
	unsigned nBytesRead;
	f_read(&File, f, sizeof f, &nBytesRead);
	f_close (&File);
	m_Screen.Write("completed\n", 10);
#endif	
	spinlock.Acquire();
	write32 (ARM_GPIO_GPFSEL0, 0x249249);
	write32 (ARM_GPIO_GPFSEL0+4, 0);
	write32 (ARM_GPIO_GPFSEL0+8, 0);
	GPIO_CLR(0xff);
//	spinlock.Release();
	memset(buffer, 0, 256*256);
	int cmd = 0;
	//int time = 0;
	data0 = 0xff;
	CString Message;
	while(1)
	{
	//	time++;
#if 0		

		if (!(GPIO & RPSPC_RST))
		{
			datain = 0;
			dataout = 0;
			cflag = p = q = 0;
			readsize = 0;
			cmd = 0;
			//Message.Format ("RESET\n");
			//m_Screen.Write ((const char *) Message, Message.GetLength ());
			while(!(GPIO & RPSPC_RST));
		}
#endif		
		if (!(GPIO & RPSPC_EXT))
		{
			a = GPIO;
			addr = (a & ADDR) >> RPSPC_A0_PIN;
			wr = (a & RPSPC_RD) != 0;
			if (a & RPSPC_A11)
			{
				if (wr)
				{
					if (addr & 1)
						tms9918_writeport1(vdp, a & 0xff);
					else
						tms9918_writeport0(vdp, a & 0xff);
					//m_Screen.Write('W');
				}
				else
				{
					GPIO_CLR(0xff);
					if (addr & 1)
						GPIO_SET(tms9918_readport1(vdp));
					else
						GPIO_SET(tms9918_readport0(vdp));
					//m_Screen.Write('R');
				}
			}
			else
			{
				switch (addr)
				{
					case 0:
						if (wr) 
						{
							datain = a & 0xff;
						}
						else
						{
							GPIO_CLR(0xff);
							GPIO_SET(data0);
						}
						break;
					case 1:
						if (!wr)
						{
							GPIO_CLR(0xff);
							GPIO_SET(dataout);
							dataout = 0;
						}
						break;
					case 2:
						if (wr)
						{
							switch (a & 0xf0)
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
											Message.Format ("cmd:0x%02x\n", datain);
											m_Screen.Write ((const char *) Message, Message.GetLength ());
										}
										q = 0;
										switch (params[0])
										{
											case SDINIT:
												buffer[0] = 100;
												Message.Format ("SDINIT\n");
												m_Screen.Write ((const char *) Message, Message.GetLength ());
												FIL File;
												Result = f_open (&File, DRIVE FILENAME, FA_READ | FA_OPEN_EXISTING);
												if (Result != FR_OK)
												{
													FileName.Format ("loading failed: %s\n", FILENAME);
													m_Screen.Write ((const char *) FileName, FileName.GetLength ());
												}
												else
												{
													m_Screen.Write ((const char *) FileName, FileName.GetLength ());
													unsigned nBytesRead;
													f_read(&File, f, sizeof f, &nBytesRead);
													f_close (&File);
													FileName.Format ("loading file: %s\n", FILENAME);
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
													Message.Format ("SDREAD(%d) %d block(s), drive %d, track %d, sector %d, %dbytes)\n", p, blocks, drv, tracks, sectors, readsize);
													m_Screen.Write ((const char *) Message, Message.GetLength ());
												}
												break;
											case SDSEND:
												memcpy(buffer, diskbuf, readsize);
												Message.Format ("SDSEND(%d) %02x %02x %02x\n", p, buffer[0],  buffer[1], buffer[2]);
												m_Screen.Write ((const char *) Message, Message.GetLength ());
												break;
											case SDCOPY:
												memcpy(fdd[params[5]]+(params[6] * 16 + (params[7]-1))*256, fdd[params[2]]+(params[3]*16+(params[4]-1))*256, 256 * params[1]);
												break;
											case SDSTATUS:
												buffer[0] = 0xc0;
												Message.Format ("SDSTATUS\n");
												m_Screen.Write ((const char *) Message, Message.GetLength ());
												break;
											case ((int)SDDRVSTS):
												buffer[0] = 0xff;
												Message.Format ("SDDRVSTS\n");
												m_Screen.Write ((const char *) Message, Message.GetLength ());
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
														strcpy(buffer, files2);
														Message.Format ("RPI_FILES: drive=%s, pattern=%s\n%s\n", drive, pattern, files2);
														m_Screen.Write ((const char *) Message, Message.GetLength ());
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
													Message.Format ("fileno:%d\n", fileno);
													m_Screen.Write ((const char *) Message, Message.GetLength ());											
													len0 = 0;
													len = 0;
													while(len0 < fileno)
													{
														if (files[len++] == '\\')
															len0++;
													}	
													len0 = 0;
													while(files[len+len0++] != '\\'); 
													memcpy(filename, "SD:/", 5);
													memcpy(filename+4, files+len, len0);
													*(filename+4+len0-1)=0;
													FILINFO fno;
													f_stat(filename, &fno);
													Message.Format ("RPI_LOAD: No.%d %s (%d)\n", fileno, filename, fno.fsize);
													m_Screen.Write ((const char *) Message, Message.GetLength ());
													Result = f_open (&File, filename, FA_READ | FA_OPEN_EXISTING);
													if (Result != FR_OK)
													{
														FileName.Format ("loading failed: %s\n", filename);
														m_Screen.Write ((const char *) FileName, FileName.GetLength ());
													}
													else
													{
														unsigned nBytesRead;
														char *point;
														if((point = strrchr(filename,'.')) != 0 ) {
															if(strcmp(point,".cas") == 0) {
																f_read(&File, buffer, fno.fsize, &nBytesRead);
																for(unsigned i = 15; i < nBytesRead; i++)
																{
																	tapbuf[i*8]   = ((buffer[i] >> 7) & 1) + '0';
																	tapbuf[i*8+1] = ((buffer[i] >> 6) & 1) + '0';
																	tapbuf[i*8+2] = ((buffer[i] >> 5) & 1) + '0';
																	tapbuf[i*8+3] = ((buffer[i] >> 4) & 1) + '0';
																	tapbuf[i*8+4] = ((buffer[i] >> 3) & 1) + '0';
																	tapbuf[i*8+5] = ((buffer[i] >> 2) & 1) + '0';
																	tapbuf[i*8+6] = ((buffer[i] >> 1) & 1) + '0';
																	tapbuf[i*8+7] = ((buffer[i] >> 0) & 1) + '0';
																}
															}
														}
														else														
															f_read(&File, tapbuf, fno.fsize, &nBytesRead);
														f_close (&File);
														FileName.Format ("loading successful: %s\n", filename);
														m_Screen.Write ((const char *) FileName, FileName.GetLength ());
														t = 0;
													}
												}
												break;
											case RPI_OLDNUM:
												buffer[0] = oldnum & 0xff;
												buffer[1] = oldnum >> 8;
												q = 0;
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
	//									data3 = q;
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
						else
						{
							GPIO_CLR(0xff);
							GPIO_SET(cflag);
						}
						break;
					case 3:
						if (!wr)
						{
							GPIO_CLR(0xff);
							GPIO_SET(data3);
						}
						else
						{
							data3 = tapbuf[t++] == '1' ? 1 : 0;	
						}
					default:
						break;
				}
			}
			while(!(GPIO & RPSPC_EXT));
		}
	}
//	spinlock.Release();
	return ShutdownReboot;
}


