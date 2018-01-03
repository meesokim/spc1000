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
#include <circle/bcm2835.h>
#include <circle/memio.h>
#include <circle/gpiopin.h>
#include <circle/timer.h>
#include <circle/util.h>

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

CSpinLock spinlock;

CKernel::CKernel (void)
:	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer)
{

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
		bOK = m_Timer.Initialize ();
	}	
	return bOK;		
}
#define GPIO (read32 (ARM_GPIO_GPLEV0))
#define GPIO_CLR(x) write32 (ARM_GPIO_GPCLR0, x)
#define GPIO_SET(x) write32 (ARM_GPIO_GPSET0, x)

TShutdownMode CKernel::Run (void)
{
	spinlock.Acquire();
	// flash the Act LED 10 times and click on audio (3.5mm headphone jack)
	unsigned int a, addr, wr, datain, dataout, cmd;
	unsigned char buffer[256*256], cflag, blocks, drive, tracks, sectors, dataread, argv;
	unsigned char *tmpbuf;
	int cmd_params[] = {0,4,4,0,7,1,1,0,4,4,0,4,4,2,6,6,0};
	int params[10], p, q;
	unsigned char* fdd[3];
	unsigned char f[] = {0x11,0x7,0xcb,0xcd,0xf3,0x07,0xc9,0x48,0x65,0x6c,0x6c,0x20,0x77,0x6f,0x72,0x6c,0x64,0x21,0x00};	
	write32 (ARM_GPIO_GPFSEL0, 0x249249);
	write32 (ARM_GPIO_GPFSEL0+4, 0);
	write32 (ARM_GPIO_GPFSEL0+8, 0);
	fdd[0] = f;
	datain = 0;
	dataout = 0;
	dataread = 0;
	cflag = p = q = 0;
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);
	m_Logger.Write (FromKernel, LogNotice, "SPC-1000 Extension");	
	GPIO_CLR(0xff);
	memset(buffer, 0, 256*256);
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
						datain = a;
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
						switch (a & 0xf0)
						{
							case rATN: // 0x80 --> 0x02
								cflag |= wRFD; 
								p = 0;
								break;
							case rDAV: // 0x10 --> 0x04
								if (!(cflag & wDAC))
								{
									cflag |= wDAC;
									
									if (p == 0) 
									{
										cmd = datain;
										argv = cmd_params[cmd];
									}
									else if (argv >= p)
										params[p] = datain;
									if (argv <= p)
									{
										q = 0;
										switch (cmd)
										{
											case SDINIT:
				//								printf("SD initialized\n");
												buffer[0] = 100;
												break;
											case SDWRITE:
												blocks = params[1];
												drive = params[2];
												tracks = params[3];
												sectors = params[4];
												tmpbuf = fdd[drive];
												if (p > argv)
													tmpbuf[p - argv] = datain;
												break;
											case SDREAD:
												tmpbuf = fdd[params[2]];
												memcpy(buffer, tmpbuf+(params[3] * 16 + (params[4]-1))*256, 256 * params[1]);
												break;
											case SDSEND:
												dataread = buffer[p];
												break;
											case SDCOPY:
												memcpy(fdd[params[5]]+(params[6] * 16 + (params[7]-1))*256, fdd[params[2]]+(params[3]*16+(params[4]-1))*256, 256 * params[1]);
												break;
											case SDSTATUS:
												buffer[0] = 0xc0;
												break;
											case SDDRVSTS:
												buffer[0] = 0xff;
												break;
											default:
												buffer[0] = cmd;
												break;
										}
									}
									else if (cmd == SDWRITE)
									{
										
									}
								}									
								break;
							case rDAC: // 0x40 --> 0x00
								if (cflag & wDAV)
								{
									q++;
									cflag &= ~wDAV;
								}
								break;
							case rRFD: // 0x20 --> 0x01
								cflag |= wDAV;
								dataout = dataread;
								p++;
								break;
							case 0: // 0x00 --> 0x00
								if (cflag & wDAC)
								{
									cflag &= ~wDAC;
									p++;
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
	for (unsigned i = 1; i <= 10; i++)
	{
		m_ActLED.On ();
		CTimer::SimpleMsDelay (200);

		m_ActLED.Off ();
		CTimer::SimpleMsDelay (500);
	}

	return ShutdownReboot;
}
