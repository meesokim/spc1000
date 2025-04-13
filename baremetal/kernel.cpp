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
#include <circle/sound/pwmsoundbasedevice.h>
#include <circle/sound/i2ssoundbasedevice.h>
#include <circle/sound/hdmisoundbasedevice.h>
#include <circle/sound/usbsoundbasedevice.h>
#include <circle/util.h>
#include "config.h"
#include "AY8910.h"
#include "casswindow.h"
#include <assert.h>
#ifdef USE_VCHIQ_SOUND
        #include <vc4/sound/vchiqsoundbasedevice.h>
#endif
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

void memset(byte *p, int length, int b);
int bpp;
static const char FromKernel[] = "kernel";
// TKeyMap spcKeyHash[0x200]; 
unsigned char keyMatrix[10];
static CKernel *s_pThis;
CKernel::CKernel(void)
	: m_Screen(320, 240),
	  m_Memory(TRUE),
	  m_Timer(&m_Interrupt),
	  m_Logger(LogPanic, &m_Timer),
// #if RASPPI <= 4
//       m_I2CMaster (CMachineInfo::Get ()->GetDevice (DeviceI2CMaster), TRUE),
// #endif
      m_USBHCI (&m_Interrupt, &m_Timer, TRUE),
#ifdef USE_VCHIQ_SOUND
      m_VCHIQ (CMemorySystem::Get (), &m_Interrupt),
#endif
	//   m_DWHCI(&m_Interrupt, &m_Timer),
	  m_ShutdownMode(ShutdownNone), 
	  m_GUI(&m_Screen)
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
	}
	// memcpy(m_Screen.GetBuffer(), samsung_bmp_c, 320*240);
	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
		if (pTarget == 0)
		{
			pTarget = &m_Screen;
		}

		bOK = m_Logger.Initialize (pTarget);
	}
	// memset(m_Screen.GetBuffer(), 0xff, 320*240);
	printf("Screen!!!\n");

	if (bOK)
	{
		// bOK = m_Interrupt.Initialize ();
	}
	printf("m_Interrupt!!!\n");

	// if (bOK)
	// {
	// 	bOK = m_DWHCI.Initialize ();
	// }                       	
	int num = 0;
	printf("m_DWHCI!!!\n");	
	// if (bOK)
	// {
	// 	bOK = m_Timer.Initialize ();
	// }
	printf("m_Timer!!!\n");	
// #if RASPPI <= 4
// 	if (bOK)
// 	{
// 			bOK = m_I2CMaster.Initialize ();
// 	}
// #endif

// 	if (bOK)
// 	{
// 			bOK = m_USBHCI.Initialize ();
// 	}	
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
	memset(spcsys.VRAM, 0x2000, 0x0);
	memset(keyMatrix, 10, 0xff);
	bpp = m_Screen.GetDepth();
	spcsys.IPLK = 1;
	spcsys.GMODE = 0;
	spcsys.cas.motor = 0;
	spcsys.cas.button = CAS_PLAY;
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
        // select the sound device
#if RASPPI <= 4
	const char *pSoundDevice = m_Options.GetSoundDevice ();
	if (strcmp (pSoundDevice, "sndpwm") == 0)
	{
			m_pSound = new CPWMSoundBaseDevice (&m_Interrupt, SAMPLE_RATE, CHUNK_SIZE);
	}
	// else if (strcmp (pSoundDevice, "sndi2s") == 0)
	// {
	// 		m_pSound = new CI2SSoundBaseDevice (&m_Interrupt, SAMPLE_RATE, CHUNK_SIZE, FALSE,
	// 											&m_I2CMaster, DAC_I2C_ADDRESS);
	// }
	else if (strcmp (pSoundDevice, "sndhdmi") == 0)
	{
			m_pSound = new CHDMISoundBaseDevice (&m_Interrupt, SAMPLE_RATE, CHUNK_SIZE);
	}
#if RASPPI >= 4
	else if (strcmp (pSoundDevice, "sndusb") == 0)
	{
			m_pSound = new CUSBSoundBaseDevice (SAMPLE_RATE);
	}
#endif
	else
	{
#ifdef USE_VCHIQ_SOUND
			m_pSound = new CVCHIQSoundBaseDevice (&m_VCHIQ, SAMPLE_RATE, CHUNK_SIZE,
									(TVCHIQSoundDestination) m_Options.GetSoundOption ());
#else
			m_pSound = new CPWMSoundBaseDevice (&m_Interrupt, SAMPLE_RATE, CHUNK_SIZE);
#endif
	}
#else
	m_pSound = new CUSBSoundBaseDevice (SAMPLE_RATE);
#endif
	//printf("Keyboard Start!\n");	
//	CCassWindow CassWindow (0, 0);
	// configure sound device
	if (!m_pSound->AllocateQueue (QUEUE_SIZE_MSECS))
	{
			// m_Logger.Write (FromKernel, LogPanic, "Cannot allocate sound queue");
	}
	m_pSound->SetWriteFormat (FORMAT, WRITE_CHANNELS);

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
			// pticks = ticks = m_Timer.GetClockTicks();
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
			// ticks = m_Timer.GetClockTicks() - ticks;
			if (!spcsys.cas.read)
				// m_Timer.usDelay(WAITTIME - (ticks < WAITTIME ? ticks : WAITTIME));
				R;
			else
				spcsys.cas.read = 0;
			//m_Timer.usDelay(ticks);
			// ticks = m_Timer.GetClockTicks();
			if (frame%1000  == 0)
			{
				//printf ("Address: %04x)", R->PC);
				//s_pThis->printf("%d, %d\n", spcsys.cycles-cycles, m_Timer.GetClockTicks() - time);
				cycles = spcsys.cycles;
				// time = m_Timer.GetClockTicks();
			}
		}
	}
	return ShutdownHalt;
}

void CKernel::KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6])
{
	CString Message;
	Message.Format ("Key status (modifiers %02X, %s)", (unsigned) ucModifiers, RawKeys);
	TKeyMap *map;
	memset(keyMatrix, 10, 0xff);
	if (ucModifiers != 0)
	{
		if ((ucModifiers & 0x10 || ucModifiers & 0x01) & (ucModifiers & 0x40 || ucModifiers & 0x4)) {
			if (RawKeys[0] == 0x4c)
				s_pThis->reset();
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
	//printf(Message);
}

int CKernel::printf(const char *format, ...)
{
	CString Message;
	va_list args;
    va_start(args, format);
	Message.FormatV(format, args);
	s_pThis->m_Screen.Write (Message, Message.GetLength());
    va_end(args);	
	return 0;
}


int ReadVal(void) 
{
	char c = 0;
	if (idx % 100 == 0)
		s_pThis->rotate (0, idx/10);
	c = tap0[idx++];
	spcsys.cas.read = 1;
	if (c == 0)
		idx = 0;
	//s_pThis->printf("%d %c\r", idx, c);
	return c-'0';
}

void OutZ80(register word Port,register byte Value)
{
	//printf("VRAM[%x]=%x\n", Port, Value);

	if ((Port & 0xE000) == 0x0000) // VRAM area
	{
		spcsys.VRAM[Port] = Value;
	}
	else if ((Port & 0xE000) == 0xA000) // IPLK area
	{
		spcsys.IPLK = !spcsys.IPLK;	// flip IPLK switch
	}
	else if ((Port & 0xE000) == 0x2000)	// GMODE setting
	{
		spcsys.GMODE = Value;
#ifdef DEBUG_MODE
		//printf("GMode:%02X\n", Value);
#endif
	}
	else if ((Port & 0xE000) == 0x6000) // SMODE
	{
		if (spcsys.cas.button != CAS_STOP)
		{

			if ((Value & 0x02)) // Motor
			{
				if (spcsys.cas.pulse == 0)
				{
					spcsys.cas.pulse = 1;

				}
			}
			else
			{
				if (spcsys.cas.pulse)
				{
					spcsys.cas.pulse = 0;
					if (spcsys.cas.motor)
					{
						spcsys.cas.motor = 0;
					}
					else
					{
						spcsys.cas.motor = 1;
					}
				}
			}
		}

//		if (spcsys.cas.button == CAS_REC && spcsys.cas.motor)
//		{
//			CasWrite(&spcsys.cas, Value & 0x01);
//		}
	}
	else if ((Port & 0xFFFE) == 0x4000) // PSG
	{

		if (Port & 0x01) // Data
		{
		    if (spcsys.psgRegNum == 15) // Line Printer
			{
				if (Value != 0)
				{
			    //spcsys.prt.bufs[spcsys.prt.length++] = Value;
				//                    printf("PRT <- %c (%d)\n", Value, Value);
				//                    printf("%s(%d)\n", spcsys.prt.bufs, spcsys.prt.length);
				}
			}
			s_pThis->ay8910.Write8910(&spcsys.ay8910, (byte) spcsys.psgRegNum, Value);
		}
		else // Reg Num
		{
			spcsys.psgRegNum = Value;
			s_pThis->ay8910.WrCtrl8910(&spcsys.ay8910, Value);
		}
	}	
}

byte InZ80(register word Port)
{
	if (Port >= 0x8000 && Port <= 0x8009) // Keyboard Matrix
	{
		return keyMatrix[Port&0xf];
	}
	else if ((Port & 0xE000) == 0xA000) // IPLK
	{
		spcsys.IPLK = !spcsys.IPLK;
	} else if ((Port & 0xE000) == 0x2000) // GMODE
	{
		return spcsys.GMODE;
	} else if ((Port & 0xE000) == 0x0000) // VRAM reading
	{
		return spcsys.VRAM[Port];
	}	else if ((Port & 0xFFFE) == 0x4000) // PSG
	{
		byte retval = 0x1f;
		if (Port & 0x01) // Data
		{
			if (spcsys.psgRegNum == 14)
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
				if (1) //(spcsys.cas.button == CAS_PLAY && spcsys.cas.motor)
				{
					if (CasRead(&spcsys.cas) == 1)
							retval |= 0x80; // high
						else
							retval &= 0x7f; // low
				}
				if (spcsys.cas.motor)
					retval &= (~(0x40)); // 0 indicates Motor On
				else
					retval |= 0x40;

			}
			else 
			{
				int data = s_pThis->ay8910.RdData8910(&spcsys.ay8910);
				//printf("r(%d,%d)\n", spcsys.psgRegNum, data);
				return data;
			}
		} else if (Port & 0x02)
		{
            retval = (ReadVal() == 1 ? retval | 0x80 : retval & 0x7f);
		}
		return retval;
	}
	return 0;
}

//#define STONE (21/48000*4000000) // 21 sample * 48000Hz 1792
#define STONE (56*32) // 21 sample * 48000Hz 1792
#define LTONE (STONE*2)

int CasRead(CassetteTape *cas)
{
	int cycles;
	int bitTime;
	
	cycles = spcsys.cycles + (count - spcsys.Z80R.ICount);
	bitTime = cycles - cas->lastTime;

	if (bitTime >= cas->bitTime)
	{
		cas->rdVal = ReadVal();
		cas->lastTime = cycles;
		cas->bitTime = cas->rdVal ? (LTONE * 0.6) : (STONE * 0.9);
		s_pThis->printf("%d%\r", idx * 100 / tapsize);		
	}

	switch (cas->rdVal)
	{
	case 0:
		if (bitTime < STONE/2)
			return 1; // high
		else
			return 0; // low

	case 1:
		if (bitTime < STONE)
			return 1; // high
		else
			return 0; // low
	}
	return 0; // low for other cases
}


void CasWrite(CassetteTape *cas, int val)
{
	Uint32 curTime;
	int t;

	t = (spcsys.cycles - cas->lastTime) >> 5;
	if (t > 100)
		cas->cnt0 = cas->cnt1 = 0;
	cas->lastTime = spcsys.cycles;
	if (cas->wrVal == 1)
	{
		if (val == 0)
			if (t > STONE/2) 
			{
//				printf("1");
//				cas->cnt0 = 0;
//				fputc('1', spconf.wfp);
			} else {
				if (cas->cnt0++ < 100)
				{
//					printf("0");
//					fputc('0', spconf.wfp);
				}
			}
	}
	cas->wrVal = val;
}

word LoopZ80(register Z80 *R) {
	return 0;
}

void PatchZ80(register Z80 *R) {
	return;
}

int printf(const char *format, ...)
{
	va_list args;
	va_start(args,format);
	CString str;
	str.FormatV(format, args);
	va_end(args);
	//s_pThis->m_Screen.Write (str, str.GetLength());
	return 1;
}	
void memset(byte *p, int length, int b)
{
	for (int i=0; i<length; i++)
		*p++ = b;
}

