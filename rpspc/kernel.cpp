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

#include "kernel.h"
#include <circle/string.h>
#include <circle/debug.h>
#include <circle/memio.h>
#include <circle/bcm2835.h>
#include <circle/gpiopin.h>
#include <circle/util.h>
#include <assert.h>

#define RPSPC_D0	(1 << 0)
#define MD00_PIN	0
#define RPSPC_WR	(1 << 14)
#define RPSPC_RD	(1 << 15)
#define RPSPC_EXT	(1 << 18)
#define RPSPC_RST   (1 << 17)
#define RPSPC_CLK	(1 << 27)
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


static const char FromKernel[] = "kernel";

CKernel::CKernel (void)
:	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Logger (m_Options.GetLogLevel ())
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
	}
	
	if (bOK)
	{
		bOK = m_Serial.Initialize (115200);
	}
	
	if (bOK)
	{
		bOK = m_Logger.Initialize (&m_Screen);
	}
	
	return bOK;
}
#define GPIO (read32 (ARM_GPIO_GPLEV0))
#define GPIO_CLR(x) write32 (ARM_GPIO_GPCLR0, x)
#define GPIO_SET(x) write32 (ARM_GPIO_GPSET0, x)

TShutdownMode CKernel::Run (void)
{
	// flash the Act LED 10 times and click on audio (3.5mm headphone jack)
	int a, addr, wr, datain, dataout, data3, readsize, data0;
	int diskbuf[258*256], buffer[256*256], cflag, blocks, drive, tracks, sectors;
	int *tmpbuf;
	int params[10], p, q;
	int* fdd[3];
	int f[256] = {0x11,0x7,0xcb,0xcd,0xf3,0x07,0xc9,0x48,0x65,0x6c,0x6c,0x20,0x77,0x6f,0x72,0x6c,0x64,0x21,0x00};	
	int args[] {0,4,4,0,7,1,1,0,4,4,0,4,4,2,6,6,0};
//	int x = 0;
	fdd[0] = f;
	datain = 0;
	dataout = 0;
	blocks = cflag = p = q = data0 = data3 = 0;
	readsize = 0;
	tmpbuf = 0;
//	CSpinLock spinlock;
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);
	m_Logger.Write (FromKernel, LogNotice, "SPC-1000 Extension");	
//	spinlock.Acquire();
	write32 (ARM_GPIO_GPFSEL0, 0x249249);
	write32 (ARM_GPIO_GPFSEL0+4, 0);
	write32 (ARM_GPIO_GPFSEL0+8, 0);
	GPIO_CLR(0xff);
//	spinlock.Release();
	memset(buffer, 0, 256*256);
	int cmd = 0;
	CString Message;
	while(1)
	{
		if (!(GPIO & RPSPC_EXT))
		{
			a = GPIO;
			addr = (a & ADDR) >> RPSPC_A0_PIN;
			wr = (a & RPSPC_RD) != 0;
			switch (addr)
			{
				case 0:
					if (wr) 
					{
						datain = GPIO & 0xff;
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
					}
					break;
				case 2:
					if (wr)
					{
						switch (GPIO & 0xf0)
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
									q = 0;
									if (args[params[0]] <= p)
									{										
										switch (params[0])
										{
											case SDINIT:
												buffer[0] = 100;
												break;
											case SDWRITE:
												if (p == 4)
												{
													blocks = params[1];
													drive = params[2];
													tracks = params[3];
													sectors = params[4];
													tmpbuf = fdd[drive] + (tracks * 16 + (sectors - 1))*256;
												} else if (p > 4)
												{
													tmpbuf[p - 5] = datain;
												}
												break;
											case SDREAD:
												if (p == 4) 
												{
													blocks = params[1];
													drive = params[2];
													tracks = params[3];
													sectors = params[4];												
													readsize = 256 * blocks;
													memcpy(diskbuf, fdd[drive]+(tracks * 16 + (sectors - 1))*256, readsize);
													Message.Format ("SDREAD(%d) %d block(s), drive %d, track %d, sector %d, %dbytes)\n", p, blocks, drive, tracks, sectors, readsize);
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
												break;
											case ((int)SDDRVSTS):
												buffer[0] = 0xff;
												break;
											default:
												buffer[0] = cmd * cmd;
												break;
										}
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
									data3 = q;
								}
								break;
							case 0: // 0x00 --> 0x00
								if (cflag & wDAC)
								{
									cflag &= ~wDAC;
									p++;
									dataout = p;
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
				default:
					break;
			}
			while(!(GPIO & RPSPC_EXT));
		}
		if (!(GPIO & RPSPC_RST))
		{
			cflag = 0;
			datain = 0;
			dataout = 0;
			while(!(GPIO & RPSPC_RST));
		}
	}
//	spinlock.Release();
	return ShutdownReboot;
}
