//
// kernel.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2018  R. Stange <rsta2@o2online.de>
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
#define __time_t_defined
#include "kernel.h"
#include <circle/string.h>

#define DRIVE		"SD:"
//#define DRIVE		"USB:"

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

#define IOSEL0 0x249249

#define GPIO_GET() (read32 (ARM_GPIO_GPLEV0))
#define GPIO_CLR(x) write32 (ARM_GPIO_GPCLR0, x)
#define GPIO_SET(x) write32 (ARM_GPIO_GPSET0, x)
#define GPIO_SEL0(x) write32(ARM_GPIO_GPFSEL0, x)

static const char FromKernel[] = "kernel";

#include "../spcbox_bm/spcbox.h"

CKernel::CKernel (void)
:	
	m_Timer (&m_Interrupt),
	m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED)
{
	m_ActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;
	m_Interrupt.Initialize ();
	m_Timer.Initialize ();
	m_EMMC.Initialize ();
	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	// Mount file system
	f_mount (&m_FileSystem, DRIVE, 1);
	SpcBox *sbox = new SpcBox();
	// m_Screen.Write ("\n", 1);
	write32 (ARM_GPIO_GPFSEL0, 0x249249);
	write32 (ARM_GPIO_GPFSEL0+4, 0);
	write32 (ARM_GPIO_GPFSEL0+8, 0);
    // write32 (BCM2835_GPIO_PADS, BCM2835_PAD_PASSWRD | BCM2835_PAD_HYSTERESIS_ENABLED | BCM2835_PAD_DRIVE_16mA);
    GPIO_CLR(0xff);
    uint8_t data[4] = {0,1,2,3};
    volatile uint32_t a;
    while(true) {
        if (!((a = GPIO_GET()) & RPSPC_EXT))
        {
            int addr = (a >> RPSPC_A0_PIN) & 3;
            if (a & RPSPC_WR) {
                GPIO_CLR(0xff);
	            GPIO_SEL0(IOSEL0);
                GPIO_SET(sbox->read(addr));
            } else {
                GPIO_SEL0(0);
                a = GPIO_GET();                
                a = GPIO_GET();                
                a = GPIO_GET();                
                a = GPIO_GET();                
                sbox->write(addr, a);
				sbox->execute();
            }
            while(!(GPIO_GET() & RPSPC_EXT));
        }
    }
	return ShutdownHalt;
}
