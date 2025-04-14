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
extern "C" {
	#include <ugui.h>
}

//#define USB_GADGET_MODE

#define DRIVE		"SD:"

static const char FromKernel[] = "kernel";

CKernel::CKernel (void)
:	m_Screen (640, 480),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer)
	// m_I2CMaster (CMachineInfo::Get ()->GetDevice (DeviceI2CMaster), TRUE),
// #ifndef USB_GADGET_MODE
// 	m_pUSB (new CUSBHCIDevice (&m_Interrupt, &m_Timer, TRUE)), // TRUE: enable plug-and-play
// #else
// 	m_pUSB (new CUSBMIDIGadget (&m_Interrupt)),
// #endif
	// m_USBHCI (&m_Interrupt, &m_Timer, TRUE),		// TRUE: enable plug-and-play
	// m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED)
	// m_pMiniOrgan (0)
{
	m_ActLED.Blink (5);	// show we are alive
	// m_pKeyboard = nullptr;
}

CKeyboard *CKeyboard::s_pThis = 0;
CKernel *CKernel::s_pThis = 0;

CMC6847 mc6847;
CKeyboard kbd;
CPU cpu;
AY8910 ay8910;
Cassette cassette;

Registers reg;
TKeyMap spcKeyHash [0x200];
unsigned char keyMatrix[16];

uint8_t memory[0x10000];

static uint8_t rb(void *userdata, uint16_t addr) {
    uint8_t data;
    data = (reg.IPLK ? ROM[addr & 0x7fff] : memory[addr&0xffff]);
    return data;
}

static void wb(void *userdata, uint16_t addr, uint8_t val) {
    memory[addr&0xffff] = val;
}

static uint8_t in(z80* const z, uint16_t port) {
    uint8_t retval = 0xff;
	if (port >= 0x8000 && port <= 0x8009) // Keyboard Matrix
	{
		return kbd.matrix(port);
	}
	else if ((port & 0xE000) == 0xA000) // IPLK
	{
		reg.IPLK = !reg.IPLK;
	} else if ((port & 0xE000) == 0x2000) // GMODE
	{
		return mc6847.GMODE;
	} else if ((port & 0xE000) == 0x0000) // VRAM reading
	{
		return mc6847.VRAM[port];
	}	
    else if ((port & 0xFFFE) == 0x4000) // PSG
	{
		if (port & 0x01) // Data
		{
			if (ay8910.reg == 14)
			{
				// 0x80 - cassette data input
				// 0x40 - motor status
				// 0x20 - print status
//				if (spcsys.prt.poweron)
//                {
//                    printf("Print Ready Check.\n");
//                    retval &= 0xcf;
//                }
//                else
//                {
//                    retval |= 0x20;
//                }
				if (cassette.motor) //(spcsys.cas.button == CAS_PLAY && spcsys.cas.motor)
				{
					retval &= (~(0x40)); // 0 indicates Motor On
                    cpu.set_turbo(1);
					if (cassette.read(cpu.getCycles(), rb(0, 0x3c5)) == 1)
							retval |= 0x80; // high
						else
							retval &= 0x7f; // low
                    // if (cpu.r->pc == 0x2ba && retval & 0x80)
                    // {
                    //     printf("%d(%d)", retval&0x80 ? 1:0, cpu.r->h);
                    // }
                    // else
                    // {
                    //     printf("0x%04x:%d\n", cpu.r->pc, retval&0x80 ? 1 : 0);
                    // }
                    // if (cassette.pos < 10)
                    // {
                    //     printf("pc:0x%04x\n", cpu.r->pc);
                    //     // cpu.debug();                        
                    // }
				}
				else
					retval |= 0x40;

			}
			else 
			{
				int data = ay8910.read();
				//printf("r(%d,%d)\n", spcsys.psgRegNum, data);
				return data;
			}
		} else if (port & 0x02)
		{
            retval = (cassette.read1() == 1 ? retval | 0x80 : retval & 0x7f);
		}
	}
    // printf("pc:%04x, port:%04x, val:%02x\n", cpu.r->pc, port, retval);
    return retval;
}

static void out(z80* const z, uint16_t port, uint8_t val) {
	if ((port & 0xE000) == 0x0000) // VRAM area
	{
		mc6847.VRAM[port&0x1fff] = val;
	}
	else if ((port & 0xE000) == 0xA000) // IPLK area
	{
		reg.IPLK = !reg.IPLK;	// flip IPLK switch
	}
	else if ((port & 0xE000) == 0x2000)	// GMODE setting
	{
		mc6847.GMODE = val;
	}
	else if ((port & 0xE000) == 0x6000) // SMODE
	{
		// if (reg.button != CASSETTE_STOP)
		{

			if ((val & 0x02)) // Motor
			{
				if (reg.pulse == 0)
				{
					reg.pulse = 1;
				}
			}
			else
			{
				if (reg.pulse)
				{
					reg.pulse = 0;
                    cassette.motor = !cassette.motor;
                    // cpu.set_turbo(cassette.motor);
                    // wb(NULL, 0x3c5, cassette.motor ? 90 : 90);
                    // printf("montor:%d(%d)\n", cassette.motor, rb(0, 0x3c5));
				}
			}
		}
        if (cassette.motor)
        {
            cassette.write(val&1);            
        }
//		if (reg.button == CAS_REC && reg.motor)
//		{
//			CasWrite(&reg, Value & 0x01);
//		}
	}
	else if ((port & 0xFFFE) == 0x4000) // PSG
	{

		if (port & 0x01) // Data
		{
		    if (ay8910.reg == 15) // Line Printer
			{
				if (val != 0)
				{
			    //spcsys.prt.bufs[spcsys.prt.length++] = Value;
				//                    printf("PRT <- %c (%d)\n", Value, Value);
				//                    printf("%s(%d)\n", spcsys.prt.bufs, spcsys.prt.length);
				}
			}
			ay8910.write(val);
            // printf("reg:%d, val:%d\n", ay8910.reg, val);
		}
		else // Reg Num
		{
			ay8910.latch(val);
            // printf("reg:%d,", val);
		}
	}
}

#define CPU_FREQ 4000000
#define PSG_CLOCK PSG_CLOCK_RATE
#define SPC1000_AUDIO_FREQ PSG_CLOCK_RATE
#define SPC1000_AUDIO_BUFFER_SIZE 512

static unsigned ptime;
static unsigned etime;
static unsigned keep_time;
// SDL_AudioSpec audioSpec;
// int audid;
bool crt_effect = true;

unsigned int GetTicks() {
	return CKernel::GetTicks();
}

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define BUFFER_WIDTH 320
#define BUFFER_HEIGHT 240
UG_COLOR crtbuf[SCREEN_HEIGHT * SCREEN_WIDTH];
UG_COLOR *pixels;

char msg[1024];
void setText(const char *s, unsigned int ktime)
{
	keep_time = GetTicks() + ktime;
	strcpy(msg, s);
}
void drawText()
{
    // textout_time = SDL_GetTicks() + keep_time;
	if (keep_time > GetTicks() && msg[0])
	{
		UG_COLOR bgcolor = 0x00000;
		UG_FillFrame(00, 00, 639, 55, 0x0);
		UG_SetForecolor(C_DARK_GRAY);
		bgcolor = C_BLACK;
		// printf("%s\n", s);
		UG_PutString(11, 15, (char *)msg);
		UG_SetForecolor(C_WHITE);
		UG_PutString(10, 14, (char *)msg);
	}
}
unsigned int execute(uint32_t interval, void* name)
{
    static int frame = 0;
    etime = GetTicks();
    cpu.pulse_irq(0);
    int steps = cpu.exec(etime);
    cpu.clr_irq();
    if (frame++%2)
	{
        mc6847.Update();
		UG_U16 * buffer = mc6847.GetBuffer();
		memcpy(pixels, crtbuf, SCREEN_WIDTH * SCREEN_HEIGHT * 2);
		for(int y = 0; y < BUFFER_HEIGHT; y++)
		{
			for(int x = 0; x < BUFFER_WIDTH; x++)
			{
				pixels[x*2] = pixels[x*2+1] = buffer[x];
			}
			pixels += SCREEN_WIDTH * 2;
		}
		drawText();
	}
    ptime = etime;
    return 0;
}

UG_GUI ug;
int w, h;
uint32_t textout_time;
char text[256];
UG_COLOR bgcolor;

void SetPixel(UG_S16 x, UG_S16 y, UG_COLOR color)
{
    if (pixels && color != bgcolor) {
        pixels[x + y * w * 2] = color;
    }
}



void clearText()
{
    // UG_FillFrame(10, 10, 640, 50, 0xff000000 | C_BLACK);
}

static uint32_t last_time = 0;

void reset()
{
    cpu.init();
    cpu.set_turbo(0);
    cpu.set_read_write(rb, wb);
    cpu.set_in_out(in, out);
    mc6847.Initialize();
    ay8910.reset();
    memset(memory, 0, 0x10000);
    reg.IPLK = true;
    last_time = GetTicks();
}

bool ProcessSpecialKey(unsigned char ucModifiers, const unsigned char RawKeys[6])
{
    bool pressed = false;
	// int index = ksym.sym % 256;
    // if (ksym.mod & KMOD_ALT)
    // {
    //     switch (ksym.sym)
    //     {
    //         case SDLK_LEFT: 
    //             cassette.prev();
    //             cassette.get_title(text);
    //             setText(text);
    //             break;
    //         case SDLK_RIGHT:
    //             cassette.next();
    //             cassette.get_title(text);
    //             setText(text);
    //             break;
    //         case SDLK_RETURN:
    //             ToggleFullscreen(screen);
    //             pressed = true;
    //             break;
    //     }
    // }
    // else {
    //     switch (ksym.sym)
    //     {
	// 		// case SDLK_F12:
	// 		// 	exit(0);
	// 		// 	break;
    //         case SDLK_F8:
    //             crt_effect = !crt_effect;
    //             break;
    //         case SDLK_F10:
    //             cpu.set_turbo(0);
    //             reset();
    //             break;
    //         case SDLK_F9:
    //             cpu.set_turbo(0);
    //             break;
    //         case SDLK_F6:
    //             SDL_Event event = {};
    //             event.type = SDL_KEYDOWN;
    //             event.key.state = SDL_PRESSED;
    //             event.key.keysym.sym = SDLK_LSHIFT;
    //             event.key.keysym.mod = 0; // from SDL_Keymod
    //             SDL_PushEvent(&event);
    //             event.key.keysym.sym = SDLK_F1;
    //             SDL_PushEvent(&event);
    //             event.type = SDL_KEYUP;
    //             event.key.state = SDL_RELEASED;
    //             event.key.keysym.sym = SDLK_F1;
    //             SDL_PushEvent(&event);
    //             event.key.keysym.sym = SDLK_LSHIFT;
    //             SDL_PushEvent(&event);
    //             break;
    //     }
    // }
    return pressed;
}

CKernel::~CKernel (void)
{
}



boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;
	// if (bOK) { bOK = m_DeviceNameService.Initialize (); }
	if (bOK) { bOK = m_Screen.Initialize (); }
	if (bOK) { bOK = m_Logger.Initialize (&m_Screen); }
	// if (bOK) { bOK = m_Interrupt.Initialize (); }
	// if (bOK) { bOK = m_Timer.Initialize (); }
	// if (bOK) { bOK = m_I2CMaster.Initialize (); }
	// if (bOK) { assert (m_pUSB); bOK = m_pUSB->Initialize (); }
	// if (bOK) { bOK = m_EMMC.Initialize (); }
	// if (bOK) { bOK = m_USBHCI.Initialize (); }
	// if (bOK) { m_pMiniOrgan = new CMiniOrgan (&m_Interrupt, &m_I2CMaster); bOK = m_pMiniOrgan->Initialize (); }
	int num = 0;
	do {
		spcKeyHash[spcKeyMap[num].sym] = spcKeyMap[num];
	} while(spcKeyMap[num++].sym != 0);       
	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);
	m_Logger.Write (FromKernel, LogNotice, "SPC-1000");
	// if (f_mount (&m_FileSystem, DRIVE, 1) != FR_OK)
	// {
	// 	m_Logger.Write (FromKernel, LogPanic, "Cannot mount drive: %s", DRIVE);
	// }
	int ncount = 0;
	// reset();
	w = 320; h = 240;
// 	UG_Init(&ug, SetPixel, SCREEN_WIDTH, SCREEN_HEIGHT);
// 	UG_FontSelect(&FONT_12X20);
// 	for(int i = 0; i < SCREEN_HEIGHT; i++)
// 	{
// 		int color = i%2 ? 0x00000000 : 0x40000000;
// 		for(int j = 0; j < SCREEN_WIDTH; j++)
// 		{
// 			crtbuf[i*640+j] = color;
// 		}
// 	}
// 	pixels = (UG_COLOR *) m_Screen.GetFrameBuffer()->GetBuffer();
// 	UG_SetBackcolor(C_BLACK);
// 	ptime = m_Timer.GetClockTicks();
// 	ay8910.initTick(ptime);
// 	cpu.initTick(ptime);
// 	cassette.initTick(ptime);
// 	cassette.get_title(text);
// 	unsigned int frames = 0;
	while(true)
	{
// 		boolean bUpdated = m_USBHCI.UpdatePlugAndPlay ();
// 		if (  bUpdated && m_pKeyboard == nullptr)
// 		{
// 			m_pKeyboard = (CUSBKeyboardDevice *) m_DeviceNameService.GetDevice ("ukbd1", FALSE);
// 			if (m_pKeyboard != nullptr)
// 			{
// 				m_pKeyboard->RegisterRemovedHandler (KeyboardRemovedHandler);

// #if 0	// set to 0 to test raw mode
// 				m_pKeyboard->RegisterShutdownHandler (ShutdownHandler);
// 				m_pKeyboard->RegisterKeyPressedHandler (KeyPressedHandler);
// #else
// 				m_pKeyboard->RegisterKeyStatusHandlerRaw (KeyStatusHandlerRaw);
// #endif
// 				m_Logger.Write (FromKernel, LogNotice, "Just type something!");			
// 			}
// 		}
// 		assert (m_pUSB);
// 		m_Screen.Rotor (0, ncount++);
// 		unsigned int time = m_Timer.GetClockTicks();
// 		if (time > ptime + 1000/60) {
// 			execute(time - ptime, 0);
// 		}
	}
	return ShutdownHalt;
}

void CKernel::KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6]) 
{
	CString Message;
	Message.Format ("Key status (modifiers %02X, %s)", (unsigned) ucModifiers, RawKeys);
	TKeyMap *map;
	memset(keyMatrix, 0xff, 10);
	if (ucModifiers != 0)
	{
		if ((ucModifiers & 0x10 || ucModifiers & 0x01) & (ucModifiers & 0x40 || ucModifiers & 0x4)) {
			if (RawKeys[0] == 0x4c)
				reset();
		}
		for(int i = 0; i < 8; i++)
			if ((ucModifiers & (1 << i)) != 0)
			{
				map = &spcKeyHash[0x100 | (1 << i)];
				if (map != 0)
					keyMatrix[map->keyMatIdx] &= ~ map->keyMask;
			}
	}

	for (unsigned i = 0; i < 6; i++)
	{
		if (RawKeys[i] != 0)
		{
			map = &spcKeyHash[RawKeys[i]];
			if (map != 0)
				keyMatrix[map->keyMatIdx] &= ~ map->keyMask;
		}
	}
	kbd.keyHandler(ucModifiers, RawKeys);
	ProcessSpecialKey(ucModifiers, RawKeys);
}


void CKernel::KeyboardRemovedHandler (CDevice *pDevice, void *pContext)
{
	assert (s_pThis != nullptr);
	CLogger::Get ()->Write (FromKernel, LogDebug, "Keyboard removed");
	// s_pThis->m_pKeyboard = 0;
}

