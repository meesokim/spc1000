//
// kernel.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014  R. Stange <rsta2@o2online.de>
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
#ifdef __cplusplus
extern "C" {
#endif
#include "Z80.h"
#include "common.h"
#include "MC6847.h"
#include "spcall.h"
#include "spckey.h"
int printf(const char *format, ...);

#ifdef __cplusplus
 }
#endif
#include "kernel.h"
#include <circle/string.h>
#include <circle/screen.h>
#include <circle/debug.h>

#include "config.h"
#include "AY8910.h"
#include "casswindow.h"
#include <assert.h>
#include <circle/util.h>

//#define SOUND_SAMPLES		(sizeof Sound / sizeof Sound[0] / SOUND_CHANNELS)

#if WRITE_FORMAT == 0
        #define FORMAT          SoundFormatUnsigned8
        #define TYPE            u8
        #define TYPE_SIZE       sizeof (u8)
        #define FACTOR          ((1 << 7)-1)
        #define NULL_LEVEL      (1 << 7)
#elif WRITE_FORMAT == 1
        #define FORMAT          SoundFormatSigned16
        #define TYPE            s16
        #define TYPE_SIZE       sizeof (s16)
        #define FACTOR          ((1 << 15)-1)
        #define NULL_LEVEL      0
#elif WRITE_FORMAT == 2
        #define FORMAT          SoundFormatSigned24
        #define TYPE            s32
        #define TYPE_SIZE       (sizeof (u8)*3)
        #define FACTOR          ((1 << 23)-1)
        #define NULL_LEVEL      0
#endif

extern char tap0[];
extern char samsung_bmp_c[];

SPCSystem spcsys;
extern TKeyMap spcKeyMap[];
TKeyMap spcKeyHash [0x200];
static int idx;

int CasRead(CassetteTape *cas);
enum colorNum {
	COLOR_BLACK, COLOR_GREEN, COLOR_YELLOW, COLOR_BLUE,
	COLOR_RED, COLOR_BUFF, COLOR_CYAN, COLOR_MAGENTA,
	COLOR_ORANGE, COLOR_CYANBLUE, COLOR_LGREEN, COLOR_DGREEN };

int bpp;
static const char FromKernel[] = "kernel";
// TKeyMap spcKeyHash[0x200]; 
unsigned char keyMatrix[10];
static CKernel *s_pThis;

CKernel::CKernel(void)
	: m_Memory(TRUE),
	  m_Options(),
	  m_DeviceNameService(),
	  m_ExceptionHandler(),
	  m_Interrupt(),
	  m_Timer(&m_Interrupt),
	  m_Logger(m_Options.GetLogLevel(), &m_Timer),
	  m_ActLED(),
	  m_Screen(320, 240),
// #if RASPPI <= 4
//       m_I2CMaster (CMachineInfo::Get ()->GetDevice (DeviceI2CMaster), TRUE),
// #endif
      m_USBHCI (&m_Interrupt, &m_Timer, TRUE),
#ifdef USE_VCHIQ_SOUND
      m_VCHIQ (CMemorySystem::Get (), &m_Interrupt),
#endif
	//   m_DWHCI(&m_Interrupt, &m_Timer),
	  m_pKeyboard (0),
	  m_GUI(&m_Screen),
	  m_ShutdownMode(ShutdownNone)
// ,m_PWMSoundDevice (&m_Interrupt)	
{
	//m_PWMSoundDevice.CancelPlayback();
	m_ActLED.Blink (5);	// show we are alive
	s_pThis = this;
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	if (bOK)
	{
		int c = 0; 	
		bOK = m_Screen.Initialize (); // BGRA
		m_Screen.SetPalette(c++, (u32)COLOR32(0x00, 0x00, 0x00, 0xff)); /* BLACK */
		m_Screen.SetPalette(c++, (u32)COLOR32(0x07, 0xff, 0x00, 0xff)); /* GREEN */ 
		m_Screen.SetPalette(c++, (u32)COLOR32(0xff, 0xff, 0x00, 0xff)); /* YELLOW */
		m_Screen.SetPalette(c++, (u32)COLOR32(0x3b, 0x08, 0xff, 0xff)); /* BLUE */
		m_Screen.SetPalette(c++, (u32)COLOR32(0xcc, 0x00, 0x3b, 0xff)); /* RED */
		m_Screen.SetPalette(c++, (u32)COLOR32(0xff, 0xff, 0xff, 0xff)); /* BUFF */
		m_Screen.SetPalette(c++, (u32)COLOR32(0x07, 0xe3, 0x99, 0xff)); /* CYAN */
		m_Screen.SetPalette(c++, (u32)COLOR32(0xff, 0x1c, 0xff, 0xff)); /* MAGENTA */
		m_Screen.SetPalette(c++, (u32)COLOR32(0xff, 0x81, 0x00, 0xff)); /* ORANGE */
		
		m_Screen.SetPalette(c++, (u32)COLOR32(0x07, 0xff, 0x00, 0xff)); /* GREEN */
		m_Screen.SetPalette(c++, (u32)COLOR32(0xff, 0xff, 0xff, 0xff)); /* BUFF */
		
		m_Screen.SetPalette(c++, (u32)COLOR32(0x00, 0x3f, 0x00, 0xff)); /* ALPHANUMERIC DARK GREEN */
		m_Screen.SetPalette(c++, (u32)COLOR32(0x07, 0xff, 0x00, 0xff)); /* ALPHANUMERIC BRIGHT GREEN */ 
		m_Screen.SetPalette(c++, (u32)COLOR32(0x91, 0x00, 0x00, 0xff)); /* ALPHANUMERIC DARK ORANGE */
		m_Screen.SetPalette(c++, (u32)COLOR32(0xff, 0x81, 0x00, 0xff)); /* ALPHANUMERIC BRIGHT ORANGE */		
		m_Screen.SetPalette(0xff,(u32)COLOR32(0xff, 0xff, 0xff, 0xff));
		m_Screen.SetPalette(0x46,(u32)COLOR32(0xff, 0x00, 0x00, 0xff));
		m_Screen.UpdatePalette();
		m_Screen.Write ("\x1b[?25l", 6);
	}
//	memcpy(m_Screen.GetBuffer(), samsung_bmp_c, 320*240);
	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
		if (pTarget == 0)
		{
			pTarget = &m_Screen;
		}

//		bOK = m_Logger.Initialize (pTarget);
	}
	// memset(m_Screen.GetBuffer(), 0xff, 320*240);
	printf("Screen!!!\n");

	if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}
	printf("m_Interrupt!!!\n");

	// if (bOK)
	// {
	// 	bOK = m_DWHCI.Initialize ();
	// }                       	
	int num = 0;
	printf("m_DWHCI!!!\n");	
	if (bOK)
	{
		bOK = m_Timer.Initialize ();
	}
	printf("m_Timer!!!\n");	
// #if RASPPI <= 4
// 	if (bOK)
// 	{
// 			bOK = m_I2CMaster.Initialize ();
// 	}
// #endif

	if (bOK)
	{
			bOK = m_USBHCI.Initialize ();
	}	
#ifdef USE_VCHIQ_SOUND
	if (bOK)
	{
			bOK = m_VCHIQ.Initialize ();
	}
#endif
	do {
		spcKeyHash[spcKeyMap[num].sym] = spcKeyMap[num];
	} while(spcKeyMap[num++].sym != 0);
	printf("spcKeyMap!!!\n");	
	
	reset();
	printf("reset!!!\n");	
	return bOK;
}

void CKernel::reset()
{
	::memset(spcsys.VRAM, 0x0, 0x2000);
	::memset(keyMatrix, 0xff, 10);
	bpp = m_Screen.GetDepth();
	spcsys.IPLK = 1;
	spcsys.GMODE = 0;
	spcsys.cas.motor = 0;
	spcsys.cas.button = CAS_PLAY;
	spcsys.cas.pulse = 0;
	spcsys.psgRegNum = 0;
	ay8910.Reset8910(&(spcsys.ay8910), 0);
	idx = 0;
	reset_flag = 1;
	spcsys.cas.lastTime = 0;
	return;
}

void CKernel::rotate(int i, int idx)
{
	m_Screen.Rotor(i, idx);
}

int CKernel::dspcallback(u32 *stream, int len) 
{
	static unsigned int nCount = 0;
	ay8910.DSPCallBack(stream, len);
	
//	memcpy(stream, (void *)(&Sound[0]+nCount), len);
//	m_Logger.Write (FromKernel, LogNoti*******ce, "Compile time: " __DATE__ " " __TIME__);
	return len;
}

int count = 0;
int tapsize = 0;
#define WAITTIME 983
Uint8 screenbuffer[320*240];
TShutdownMode CKernel::Run (void)
{
	int frame = 0, ticks = 0, pticks = 0, d = 0;
	unsigned int t = 0;
	int cycles = 0;	
	int time = 0;
	tapsize = strlen(tap0);
	CString Message;
	//m_PWMSound.Play(this);//, SOUND_CHANNELS, SOUND_BITS,Sound, SOUND_SAMPLES );
	InitMC6847(m_Screen.GetBuffer(), &spcsys.VRAM[0], 256,192);	
	//m_PWMSound.Playback (Sound, SOUND_SAMPLES, SOUND_CHANNELS, SOUND_BITS);
	// m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);
        // Sound device disabled

#if 0
	CUSBKeyboardDevice *pKeyboard = (CUSBKeyboardDevice *) m_DeviceNameService.GetDevice ("ukbd1", FALSE);
	if (pKeyboard == 0)
	{
	//	m_Logger.Write (FromKernel, LogError, "Keyboard not found");
	} 
	else
	{
		pKeyboard->RegisterKeyStatusHandlerRaw (KeyStatusHandlerRaw); 		
	}
#endif	
	Z80 *R = &spcsys.Z80R;	
	reset_flag = 1;
	while(1)
	{
		if (reset_flag) {
			ResetZ80(R);
			R->ICount = I_PERIOD;
			ticks = m_Timer.GetClockTicks();
			spcsys.cycles = 0;	
			reset_flag = 0;
		}
		count = R->ICount;
		ExecZ80(R); // Z-80 emulation
		spcsys.cycles += (count - R->ICount);
		if (R->ICount <= 0)
		{
			frame++;
			spcsys.tick++;		// advance spc tick by 1 msec
			R->ICount += I_PERIOD;	// re-init counter (including compensation)

			// Poll USB plug-and-play periodically (approx. once per second) to detect keyboard
			if (frame % 60 == 0)
			{
				m_USBHCI.UpdatePlugAndPlay ();
				if (m_pKeyboard == 0)
				{
					m_pKeyboard = (CUSBKeyboardDevice *) m_DeviceNameService.GetDevice ("ukbd1", FALSE);
					if (m_pKeyboard != 0)
					{
						m_pKeyboard->RegisterKeyStatusHandlerRaw (KeyStatusHandlerRaw);
					}
				}
			}

			if (frame % 16 == 0)
			{
				if (R->IFF & IFF_EI)	// if interrupt enabled, call Z-80 interrupt routine
				{
					R->IFF |= IFF_IM1 | IFF_1;
					IntZ80(R, 0);
				}
			}
			if (frame%33 == 0)
			{
				Update6847(spcsys.GMODE);
				// m_GUI.Update();
				R->ICount -= 20;
			}
			ay8910.Loop8910(&spcsys.ay8910, 1);

			// System Timer Frame Delay to maintain correct virtual Z80 execution speed
			ticks = m_Timer.GetClockTicks() - ticks;
			if (!spcsys.cas.read)
			{
				m_Timer.usDelay(WAITTIME - (ticks < WAITTIME ? ticks : WAITTIME));
			}
			else
				spcsys.cas.read = 0;
			//m_Timer.usDelay(ticks);
			// ticks = m_Timer.GetClockTicks();
			if (frame%1000  == 0)
			{
				//printf ("Address: %04x)", R->PC);
				//s_pThis->printf("%d, %d\n", spcsys.cycles-cycles, m_Timer.GetClockTicks() - time);
			}
			ticks = m_Timer.GetClockTicks();
		}
	}

	return ShutdownHalt;
}

void CKernel::ShutdownHandler (void)
{
	//s_pThis->m_ShutdownMode = ShutdownReboot;
}

void CKernel::KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6])
{
	// Detect ALT + LEFT/RIGHT cassette tape controls
	bool alt_pressed = (ucModifiers & 0x44) != 0; // Left Alt (0x04) or Right Alt (0x40)
	if (alt_pressed)
	{
		bool left_pressed = false;
		bool right_pressed = false;
		for (int r = 0; r < 6; r++)
		{
			if (RawKeys[r] == 0x50) left_pressed = true; // CRLK_LEFT
			if (RawKeys[r] == 0x4f) right_pressed = true; // CRLK_RIGHT
		}

		static bool left_was_pressed = false;
		static bool right_was_pressed = false;

#if 0
		if (left_pressed && !left_was_pressed)
		{
			cassette.prev();
			if (s_pThis)
			{
				char title[64];
				cassette.get_title(title);
				s_pThis->show_osd(title);
			}
		}
		if (right_pressed && !right_was_pressed)
		{
			cassette.next();
			if (s_pThis)
			{
				char title[64];
				cassette.get_title(title);
				s_pThis->show_osd(title);
			}
		}
#endif
		left_was_pressed = left_pressed;
		right_was_pressed = right_pressed;

		// Consume keypress so it is not forwarded to Z80
		if (left_pressed || right_pressed)
		{
			return;
		}
	}

	// Reset entire keyMatrix to all 0xFF (not pressed)
	::memset(keyMatrix, 0xff, 10);

	// Map modifier bits (Shift, Ctrl, Alt/Graph)
	for (int i = 0; spcKeyMap[i].keyMatIdx != -1; i++)
	{
		if (spcKeyMap[i].sym & 0x100) // Modifier key syms have 0x100 ORed
		{
			unsigned char mod_bit = spcKeyMap[i].sym & 0xFF;
			if (ucModifiers & mod_bit)
			{
				keyMatrix[spcKeyMap[i].keyMatIdx] &= ~spcKeyMap[i].keyMask;
			}
		}
	}

	// Map raw keys (up to 6 keys pressed simultaneously)
	for (int r = 0; r < 6; r++)
	{
		unsigned char k = RawKeys[r];
		if (k == 0) continue;

		for (int i = 0; spcKeyMap[i].keyMatIdx != -1; i++)
		{
			if (!(spcKeyMap[i].sym & 0x100) && spcKeyMap[i].sym == k)
			{
				keyMatrix[spcKeyMap[i].keyMatIdx] &= ~spcKeyMap[i].keyMask;
				break;
			}
		}
	}
}

int CKernel::printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int nResult = vprintf(format, args);
	va_end(args);
	return nResult;
}

int CKernel::vprintf(const char *format, va_list args)
{
	if (s_pThis == 0) return 0;

	CString Message;
	Message.FormatV(format, args);

	s_pThis->m_Logger.Write(FromKernel, LogNotice, Message);

	return 0;
}

extern "C" int printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int result = CKernel::vprintf(format, args);
	va_end(args);
	return result;
}

extern "C" byte InZ80(word Port)
{
	if (Port >= 0x8000 && Port <= 0x8009) // Keyboard Matrix
	{
		return keyMatrix[Port-0x8000];
	}
	else if ((Port & 0xE000) == 0xA000) // IPLK
	{
		spcsys.IPLK = (spcsys.IPLK)? 0 : 1;
	}
	else if ((Port & 0xE000) == 0x2000) // GMODE
	{
		return spcsys.GMODE;
	}
	else if ((Port & 0xE000) == 0x0000) // VRAM reading
	{
		return spcsys.VRAM[Port];
	}
	else if ((Port & 0xFFFE) == 0x4000) // PSG
	{
		byte retval = 0x1f;
		if (Port & 0x01) // Data
		{
			if (spcsys.psgRegNum == 14)
			{
				if (spcsys.cas.motor)
					retval &= (~(0x40)); // 0 indicates Motor On
				else
					retval |= 0x40;
			}
			else 
			{
				if (s_pThis) return s_pThis->ay8910.RdData8910(&spcsys.ay8910);
			}
		}
		return retval;
	}
	return 0xff;
}

extern "C" void OutZ80(word Port,byte Value)
{
	if (Port < 0x2000) // VRAM area
	{
		spcsys.VRAM[Port] = Value;
	}
	else if ((Port & 0xE000) == 0xA000) // IPLK area
	{
		spcsys.IPLK = (spcsys.IPLK)? 0 : 1;	// flip IPLK switch
	}
	else if ((Port & 0xE000) == 0x2000)	// GMODE setting
	{
		spcsys.GMODE = Value;
	}
	else if ((Port & 0xE000) == 0x6000) // SMODE
	{
		if (spcsys.cas.button != CAS_STOP)
		{
			if ((Value & 0x02)) // Motor
			{
				if (spcsys.cas.pulse == 0) spcsys.cas.pulse = 1;
			}
			else
			{
				if (spcsys.cas.pulse)
				{
					spcsys.cas.pulse = 0;
					spcsys.cas.motor = !spcsys.cas.motor;
				}
			}
		}
	}
	else if ((Port & 0xFFFE) == 0x4000) // PSG
	{
		if (s_pThis)
		{
			if (Port & 0x01) s_pThis->ay8910.Write8910(&spcsys.ay8910, (byte) spcsys.psgRegNum, Value);
			else s_pThis->ay8910.WrCtrl8910(&spcsys.ay8910, Value);
		}
	}
}

extern "C" void PatchZ80( Z80 *R) {}
extern "C" word LoopZ80( Z80 *R) { return INT_NONE; }
