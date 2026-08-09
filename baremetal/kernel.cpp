#include "kernel.h"
#include <circle/timer.h>
#include <circle/bcmframebuffer.h>
#include <circle/util.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

extern "C" {
#include "Z80.h"
#include "MC6847.h"
#include "spcall.h"
#include "common.h"
#include "spckey.h"
#include "tape_loader.h"
}

// 8bpp intermediate buffer for MC6847
static u8 mc6847_buf[320*240];
static u16 pal565[256];

SPCSystem spcsys;
extern unsigned char ROM[32768];

// Embedded tape data (from tap.c)
extern char tap0[];
static int tapeLen = 0;
static int tapePos = 0;
static TapeLoaderConfig tapeCfg;

// Enable to log tape-loading debug messages (per-bit reads, screen dumps,
// header/RAM dumps) to SD:/log.txt. Disabled by default.
// #define CASSETTE_DEBUG_MESSAGE

#ifdef CASSETTE_DEBUG_MESSAGE
static void WriteLog(const char *format, ...)
{
    FIL File;
    FRESULT res = f_open (&File, "SD:/log.txt", FA_WRITE | FA_OPEN_ALWAYS);
    if (res == FR_OK)
    {
        f_lseek(&File, f_size(&File));
        char buf[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        
        unsigned int nWritten = 0;
        f_write(&File, buf, strlen(buf), &nWritten);
        f_close(&File);
    }
#ifdef HOST_COMPILE
    else
    {
        fprintf(stderr, "WriteLog: f_open failed with %d\n", res);
    }
#endif
}
#endif

// Enable to log tape-loading debug messages (per-bit reads, screen dumps,
// header/RAM dumps) to SD:/log.txt. Disabled by default.
// #define CASSETTE_DEBUG_MESSAGE

// Enable to draw emulator status overlays (tape title, motor state, header
// info, reset message) into VRAM rows. Disabled by default so the display
// matches real hardware behavior.
// #define SCREEN_DEBUG_MESSAGE

#ifdef SCREEN_DEBUG_MESSAGE
static void ScreenLog(int row, const char *format, ...)
{
    char buf[33];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    
#ifdef HOST_COMPILE
    // printf("[ScreenLog Row %02d] %s\n", row, buf);
    // fflush(stdout);
#endif

    int len = 0;
    while (buf[len] && len < 32)
    {
        spcsys.VRAM[row * 32 + len] = buf[len];
        len++;
    }
    while (len < 32)
    {
        spcsys.VRAM[row * 32 + len] = ' ';
        len++;
    }
}
#else
static void ScreenLog(int row, const char *format, ...)
{
    // No-op: overlay hidden to match real hardware display.
    (void)row;
    (void)format;
}
#endif

#ifdef CASSETTE_DEBUG_MESSAGE
static int loaded_byte_count = 0;

static void DumpScreen(void)
{
    WriteLog("=== SCREEN BUFFER DUMP ===\n");
    for (int r = 0; r < 16; r++)
    {
        char line[33];
        for (int c = 0; c < 32; c++)
        {
            byte val = spcsys.VRAM[r * 32 + c];
            line[c] = (val >= 32 && val < 127) ? val : '.';
        }
        line[32] = '\0';
        WriteLog("%s\n", line);
#ifdef HOST_COMPILE
        // printf("%s\n", line);
#endif
    }
    WriteLog("==========================\n");
#ifdef HOST_COMPILE
    // printf("==========================\n");
    // fflush(stdout);
#endif
}
#endif

// Cassette tape is handled via m_Cassette (Cassette class member of CKernel)


// SD card configuration loader for tape settings
static bool LoadTapeConfig(void)
{
    FIL File;
    UINT nBytesRead = 0;
    char *buf = NULL;
    bool ok = false;

    TapeLoaderConfig_InitDefaults(&tapeCfg);

    if (f_open(&File, "SD:/spcconfig.ini", FA_READ | FA_OPEN_EXISTING) == FR_OK)
    {
        DWORD size = f_size(&File);
        if (size > 0 && size < 8192)
        {
            buf = new char[size + 1];
            if (buf)
            {
                if (f_read(&File, buf, size, &nBytesRead) == FR_OK && nBytesRead == size)
                {
                    buf[size] = '\0';
                    ok = TapeLoaderConfig_Parse(&tapeCfg, buf);
                }
                delete[] buf;
            }
        }
        f_close(&File);
    }

    return ok;
}

// Cassette state (cycles-based timing matching sdl2/cassette.cpp)
static unsigned int batch_start_cycles = 0;
static int batch_start_icount = 0;

static unsigned int GetCycles(void)
{
    return batch_start_cycles + (batch_start_icount - spcsys.Z80R.ICount);
}

static unsigned int casLastTime = 0;
static int casReadVal = 0;
static int consecutiveZeros = 0;

class CKernel; // forward
static CKernel *s_pThis;

static int ReadBitFrom(char *tapBuf, int tapLen, int &tapPos)
{
    if (tapLen == 0)
    {
        int len = 0;
        while (tapBuf[len]) len++;
        tapLen = len;
    }

    if (tapPos >= tapLen)
    {
        return 0;
    }

    int c;
    int zero_skip = tapeCfg.zero_skip;
    if (zero_skip < 1) zero_skip = 1;

    if (consecutiveZeros > zero_skip)
    {
        while (tapPos < tapLen && tapBuf[tapPos] == '0')
        {
            tapPos++;
        }
        if (tapPos < tapLen)
        {
            c = (tapBuf[tapPos++] == '1' ? 1 : 0);
        }
        else
        {
            c = 0;
        }
        consecutiveZeros = 0;
    }
    else
    {
        c = (tapBuf[tapPos++] == '1' ? 1 : 0);
    }

    if (c == 0)
    {
        consecutiveZeros++;
    }
    else
    {
        consecutiveZeros = 0;
    }

    return c;
}

static int ReadTapeBit(void)
{
    return ReadBitFrom(tap0, tapeLen, tapePos);
}

static int CasRead(void)
{
	if (!spcsys.cas.motor)
		return 1;
	if (s_pThis && s_pThis->m_Cassette.get_len() > 0)
	{
		// SD card path: read one bit per poll from the Cassette buffer,
		// matching the ROM's polling model (not the old cycle-timing model).
		int bit = ReadBitFrom(s_pThis->m_Cassette.get_tape(),
		                      s_pThis->m_Cassette.get_len(),
		                      s_pThis->m_Cassette.pos);
#ifdef CASSETTE_DEBUG_MESSAGE
		static int casread_count = 0;
		if (casread_count < 500)
		{
			WriteLog("[CasRead Cassette] %d: pc=0x%04X bit=%d pos=%d zeros=%d motor=%d\n",
				casread_count++, spcsys.Z80R.PC.W, bit, s_pThis->m_Cassette.pos, consecutiveZeros, spcsys.cas.motor);
		}
#endif
		return bit;
	}
	int bit = ReadTapeBit();
#ifdef CASSETTE_DEBUG_MESSAGE
	static int casread_count = 0;
	if (casread_count < 1000)
	{
		WriteLog("[CasRead Tap0] %d: pc=0x%04X bit=%d tapePos=%d zeros=%d motor=%d\n",
			casread_count++, spcsys.Z80R.PC.W, bit, tapePos, consecutiveZeros, spcsys.cas.motor);
	}
#endif
	return bit;
}

TKeyMap spcKeyHash[0x200];
unsigned char keyMatrix[10] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
volatile bool g_reset_requested = false;
volatile bool g_ipl_reset = false;

// Set from the USB keyboard interrupt handler; consumed in the main loop.
volatile int tape_switch_request = 0;
static bool left_was_pressed = false;
static bool right_was_pressed = false;

int bpp = 16;
char samsung_bmp_c[] = "";

#define RGB565(r,g,b) (((r)&0x1F)<<11 | ((g)&0x3F)<<5 | ((b)&0x1F))
#define I_PERIOD 4000
#define SAMPLE_RATE   44100
#define PSG_CLOCK    1789776

// CSPCSoundDevice: generate PSG samples directly, no queue needed
unsigned CSPCSoundDevice::GetChunk (s16 *pBuffer, unsigned nChunkSize)
{
	PSG *psg = *m_ppPSG;
	if (!psg) return 0;

	// nChunkSize is number of s16 words; stereo = 2 per frame
	for (unsigned i = 0; i < nChunkSize; i += 2)
	{
		s16 val = PSG_calc(psg);
		pBuffer[i]     = val;  // L
		pBuffer[i + 1] = val;  // R (mono)
	}
	return nChunkSize;
}

CKernel::CKernel (void)
:	m_Memory (TRUE),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_Screen (320, 240),
	m_VCHIQ (CMemorySystem::Get (), &m_Interrupt),
	m_USBHCI (&m_Interrupt, &m_Timer, TRUE),
	m_pKeyboard (0),
	m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED),
	m_pSound (0),
	m_pPSG (0)
{
	m_ActLED.Blink (5);
	s_pThis = this;
}

CKernel::~CKernel (void)
{
	delete m_pSound;
	if (m_pPSG) PSG_delete(m_pPSG);
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	bOK = m_Screen.Initialize ();
	if (!bOK) return FALSE;

	// Hide cursor and set text color to black (invisible on black bg)
	m_Screen.Write ("\x1b[?25l", 6);

	if (bOK) bOK = m_Interrupt.Initialize ();
	if (bOK) bOK = m_Timer.Initialize ();
	if (bOK) bOK = m_USBHCI.Initialize ();
	if (bOK)
	{
		if (m_EMMC.Initialize ())
		{
			f_mount (&m_FileSystem, "SD:", 0);
		}
	}

	if (bOK)
	{
		// Logger not connected to screen - no boot text/cursor
		m_Logger.Initialize (0);
	}

	// Palette
	pal565[0]  = RGB565(0x00>>3, 0x00>>2, 0x00>>3);
	pal565[1]  = RGB565(0x07>>3, 0xff>>2, 0x00>>3);
	pal565[2]  = RGB565(0xff>>3, 0xff>>2, 0x00>>3);
	pal565[3]  = RGB565(0x3b>>3, 0x08>>2, 0xff>>3);
	pal565[4]  = RGB565(0xcc>>3, 0x00>>2, 0x3b>>3);
	pal565[5]  = RGB565(0xff>>3, 0xff>>2, 0xff>>3);
	pal565[6]  = RGB565(0x07>>3, 0xe3>>2, 0x99>>3);
	pal565[7]  = RGB565(0xff>>3, 0x1c>>2, 0xff>>3);
	pal565[8]  = RGB565(0xff>>3, 0x80>>2, 0x00>>3);
	pal565[9]  = RGB565(0x3b>>3, 0x08>>2, 0xff>>3);
	pal565[10] = RGB565(0x07>>3, 0xe3>>2, 0x99>>3);
	pal565[11] = RGB565(0x00>>3, 0x00>>2, 0x00>>3);
	pal565[12] = RGB565(0x07>>3, 0xff>>2, 0x00>>3);
	pal565[13] = RGB565(0xcc>>3, 0x00>>2, 0x3b>>3);
	pal565[14] = RGB565(0xff>>3, 0x80>>2, 0x00>>3);
	pal565[15] = RGB565(0xff>>3, 0xff>>2, 0x00>>3);
	for (int i = 16; i < 256; i++) pal565[i] = pal565[i % 16];

	memcpy(spcsys.ROM, ROM, 0x8000);
	// Load tape loader configuration
	LoadTapeConfig();
	// Initialize tapeLen from tap0 fallback to resolve the tape presence detection chicken-and-egg bug
	{
		int len = 0;
		while (tap0[len]) len++;
		tapeLen = len;
	}
	spcsys.IPLK = 1;
	spcsys.GMODE = 0;
	memset(spcsys.VRAM, 0, 0x2000);
	memset(spcsys.RAM, 0, 0x10000);
	memset(keyMatrix, 0xff, 10);
	spcsys.psgRegNum = 0;
	spcsys.cas.button = 1; // CAS_PLAY
	spcsys.cas.motor = 0;
	spcsys.cas.pulse = 0;

	// Create emu2149 PSG
	m_pPSG = PSG_new(PSG_CLOCK, SAMPLE_RATE);
	if (m_pPSG)
	{
		PSG_setVolumeMode(m_pPSG, EMU2149_VOL_AY_3_8910);
		PSG_set_quality(m_pPSG, 0);
		PSG_reset(m_pPSG);
	}

	// VCHIQ init
	if (bOK)
	{
		bOK = m_VCHIQ.Initialize();
		for (int i = 0; i < 10; i++)
			m_Scheduler.Yield();
	}

	// VCHIQ sound with direct GetChunk
	m_pSound = new CSPCSoundDevice(&m_VCHIQ, &m_pPSG, SAMPLE_RATE);
	if (m_pSound)
	{
		m_pSound->Start();
		for (int i = 0; i < 20; i++)
			m_Scheduler.MsSleep(1);
	}

	// Key hash
	int num = 0;
	do {
		spcKeyHash[spcKeyMap[num].sym] = spcKeyMap[num];
	} while(spcKeyMap[num++].sym != 0);

	return bOK;
}

TShutdownMode CKernel::Run (void){
	InitMC6847(mc6847_buf, spcsys.VRAM, 256, 192);

	CBcmFrameBuffer *pFB = m_Screen.GetFrameBuffer();
	u16 *pScreen = (u16 *)(uintptr)pFB->GetBuffer();
	unsigned sw = pFB->GetWidth();
	unsigned sh = pFB->GetHeight();
	unsigned pitch = pFB->GetPitch() / 2;
	unsigned offX = (sw > 320) ? (sw - 320) / 2 : 0;
	unsigned offY = (sh > 240) ? (sh - 240) / 2 : 0;

	// Load cassette directory from SD card
	m_Cassette.loaddir("SD:/taps");
	{
		char title[256];
		m_Cassette.get_title(title);
		if (title[0]) ScreenLog(10, "Tape: %s", title);
	}

	Z80 *R = &spcsys.Z80R;
	ResetZ80(R);
	R->ICount = I_PERIOD;

	int frame = 0;

	while (1)
	{
		if (g_reset_requested)
		{
			g_reset_requested = false;
			spcsys.IPL_SW = g_ipl_reset ? 1 : 0;
			memcpy(spcsys.ROM, ROM, 0x8000);
			spcsys.IPLK = 1;
			ResetZ80(R);
			memset(keyMatrix, 0xff, 10);
			tapePos = 0;
			consecutiveZeros = 0;
			casReadVal = 0;
			spcsys.cas.motor = 0;
			spcsys.cas.pulse = 0;
			spcsys.cycles = 0;
			if (s_pThis)
			{
				s_pThis->m_Cassette.motor = 0;
				// Only rewind the tape on reset if rewind_on_reset is enabled.
				// Otherwise a multi-block tape keeps its position so the user can
				// continue loading subsequent blocks after a warm reset.
				if (tapeCfg.rewind_on_reset)
				{
					s_pThis->m_Cassette.pos = 0;
				}
			}
			ScreenLog(13, "RESET SYSTEM (%s)", g_ipl_reset ? "IPL" : "NORMAL");
		}

		int count = R->ICount;
		batch_start_cycles = spcsys.cycles;
		batch_start_icount = R->ICount;
		ExecZ80(R);
		spcsys.cycles += (count - R->ICount);



		if (R->ICount <= 0)
		{
			frame++;
			spcsys.tick++;
			R->ICount += I_PERIOD;

#ifdef CASSETTE_DEBUG_MESSAGE
			if (frame % 300 == 0)
			{
				DumpScreen();
			}

			// Detect BREAK on screen and log state
			static bool break_logged = false;
			if (!break_logged)
			{
				const char *p = (const char *)spcsys.VRAM;
				bool has_break = (strstr(p, "BREAK") != 0);
				if (has_break)
				{
					break_logged = true;
					WriteLog("=== BREAK DETECTED at frame %d ===\n", frame);
					WriteLog("Z80 PC=0x%04X, tapePos=%d, consecutiveZeros=%d, casReadVal=%d\n",
						spcsys.Z80R.PC.W, tapePos, consecutiveZeros, casReadVal);
					WriteLog("BASIC text RAM dump 0x7C4E-0x7D4D:");
					for (int i = 0; i < 0x100; i++) WriteLog(" %02X", spcsys.RAM[0x7C4E + i]);
					WriteLog("\n");
					DumpScreen();
				}
			}
#endif

			// Deferred cassette prev()/next() (requested from the keyboard
			// interrupt handler; run here in the main loop, not in IRQ context)
			if (tape_switch_request != 0 && s_pThis)
			{
				int dir = tape_switch_request;
				tape_switch_request = 0;
				if (dir < 0)
				{
					s_pThis->m_Cassette.prev();
				}
				else
				{
					s_pThis->m_Cassette.next();
				}
				char title[256];
				s_pThis->m_Cassette.get_title(title);
				ScreenLog(10, "Tape: %s", title);
			}

			if (frame % 60 == 0)
			{
				m_USBHCI.UpdatePlugAndPlay();
				if (m_pKeyboard == 0)
				{
					m_pKeyboard = (CUSBKeyboardDevice *)
						m_DeviceNameService.GetDevice("ukbd1", FALSE);
					if (m_pKeyboard != 0)
						m_pKeyboard->RegisterKeyStatusHandlerRaw(KeyStatusHandlerRaw);
				}
			}

#ifdef HOST_COMPILE
			// Automated LOAD command key sequence injection (starts at frame 1100)
			static int autotype_step = -1;
			if (frame >= 1100 && autotype_step < 10)
			{
				int new_step = (frame - 1100) / 60;
				if (new_step != autotype_step)
				{
					autotype_step = new_step;
					unsigned char raw_keys[6] = {0};
					switch (autotype_step) {
						case 0: raw_keys[0] = 0x0F; break; // 'L'
						case 1: break;                       // release
						case 2: raw_keys[0] = 0x12; break; // 'O'
						case 3: break;                       // release
						case 4: raw_keys[0] = 0x04; break; // 'A'
						case 5: break;                       // release
						case 6: raw_keys[0] = 0x07; break; // 'D'
						case 7: break;                       // release
						case 8: raw_keys[0] = 0x28; break; // RETURN
						case 9: break;                       // release
					}
					if (autotype_step <= 9)
					{
						KeyStatusHandlerRaw(0, raw_keys);
					}
				}
			}
#endif

			if (frame % 16 == 0)
			{
				if (R->IFF & IFF_EI)
				{
					R->IFF |= IFF_IM1 | IFF_1;
					IntZ80(R, 0);
				}
			}

			if (frame % 33 == 0)
			{
				Update6847(spcsys.GMODE);
				for (unsigned y = 0; y < 240; y++)
				{
					u16 *dst = pScreen + (y + offY) * pitch + offX;
					u8  *src = mc6847_buf + y * 320;
					for (unsigned x = 0; x < 320; x++)
						dst[x] = pal565[src[x]];
				}
				R->ICount -= 20;
			}

			if (spcsys.cas.motor)
			{
				if (frame % 200 == 0)
				{
					m_Scheduler.Yield();
				}
			}
			else
			{
				m_Scheduler.MsSleep(1);
			}
		}
	}

	return ShutdownHalt;
}

void CKernel::KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6])
{
	// Detect ALT + LEFT/RIGHT cassette tape controls.
	// NOTE: This is called from the USB interrupt handler. We only record
	// the requested direction here; the actual prev()/next() call (which
	// allocates heap and reads the SD card) is deferred to the main loop.
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

		if (left_pressed && !left_was_pressed)
		{
			tape_switch_request = -1;
		}
		if (right_pressed && !right_was_pressed)
		{
			tape_switch_request = 1;
		}

		left_was_pressed = left_pressed;
		right_was_pressed = right_pressed;

		// Consume keypress so it is not forwarded to Z80
		if (left_pressed || right_pressed)
		{
			return;
		}
	}

	memset(keyMatrix, 0xff, 10);
	for (int i = 0; spcKeyMap[i].keyMatIdx != -1; i++)
	{
		if (spcKeyMap[i].sym & 0x100)
		{
			if (ucModifiers & (spcKeyMap[i].sym & 0xFF))
				keyMatrix[spcKeyMap[i].keyMatIdx] &= ~spcKeyMap[i].keyMask;
		}
	}
	for (int r = 0; r < 6; r++)
	{
		unsigned char k = RawKeys[r];
		if (k == 0) continue;
		if (k == 0x45 || k == 0x43) // F12 or F10
		{
			g_reset_requested = true;
			g_ipl_reset = (ucModifiers & 0x44) ? true : false;
			continue;
		}
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

// Z80 I/O callbacks
extern "C" {

void PatchZ80(Z80 *R) {}
word LoopZ80(Z80 *R) { return INT_NONE; }

byte InZ80(word Port)
{
	if (Port >= 0x8000 && Port <= 0x8009)
	{
		byte val = keyMatrix[Port - 0x8000];
		// WriteLog("[InZ80 Keyboard] Port=0x%04X, val=0x%02X\n", Port, val);
#ifdef HOST_COMPILE
		// printf("[InZ80 Keyboard] Port=0x%04X, val=0x%02X\n", Port, val);
		// fflush(stdout);
#endif
		return val;
	}
	else if ((Port & 0xE000) == 0xA000)
		spcsys.IPLK = spcsys.IPLK ? 0 : 1;
	else if ((Port & 0xE000) == 0x2000)
		return spcsys.GMODE;
	else if ((Port & 0xE000) == 0x0000)
		return spcsys.VRAM[Port];
	else if ((Port & 0xFFFE) == 0x4000) // PSG
	{
		if (Port & 0x01)
		{
			if (spcsys.psgRegNum == 14)
			{
				byte r = 0xff;
				if (spcsys.cas.button == 1 && spcsys.cas.motor) // CAS_PLAY && motor on
				{
					word pc = spcsys.Z80R.PC.W;
					if (pc >= 0x02E0 && pc <= 0x0370)
					{
						r |= 0x80; // Return dummy high bit during motor check to avoid stealing tape bits
					}
					else
					{
						if (CasRead() == 1) r |= 0x80;
						else r &= 0x7f;
					}
					r &= ~0x40; // Motor On (0)
				}
				else
				{
					r |= 0x40;  // Motor Off (1)
				}
				return r;
			}
			if (s_pThis && s_pThis->m_pPSG)
				return PSG_readIO(s_pThis->m_pPSG);
		}
		return 0x1f;
	}
	else if (Port == 0x4003)
	{
		return (s_pThis && s_pThis->m_Cassette.get_len() > 0) ? 1 : (tapeLen > 0 ? 1 : 0);
	}
	else if (Port == 0x4004)
	{
		// Byte-oriented read via CasRead (uses m_Cassette internally)
		return 0xff;
	}
	return 0xff;
}

void OutZ80(word Port, byte Value)
{
	if (Port < 0x2000)
		spcsys.VRAM[Port] = Value;
	else if ((Port & 0xE000) == 0xA000)
		spcsys.IPLK = spcsys.IPLK ? 0 : 1;
	else if ((Port & 0xE000) == 0x2000)
		spcsys.GMODE = Value;
	else if ((Port & 0xE000) == 0x6000) // cassette motor
	{
		if (spcsys.cas.button != 0)
		{
			if (Value & 0x02)
			{
				if (spcsys.cas.pulse == 0) spcsys.cas.pulse = 1;
			}
			else
			{
				if (spcsys.cas.pulse)
				{
					spcsys.cas.pulse = 0;
					spcsys.cas.motor = !spcsys.cas.motor;
					if (s_pThis) s_pThis->m_Cassette.motor = spcsys.cas.motor;
						if (spcsys.cas.motor)
						{
							casLastTime = GetCycles();
							consecutiveZeros = 0;
							casReadVal = 0;
#ifdef CASSETTE_DEBUG_MESSAGE
							loaded_byte_count = 0; // Reset byte counter
#endif
							if (s_pThis)
							{
								s_pThis->m_Cassette.initTick(GetCycles());
								// Do NOT reset pos here: multi-block tapes keep playing
								// forward, so the next block is read where the last
								// one ended. (Tape rewind happens only on system reset
								// when rewind_on_reset is enabled.)
							}
							if (CActLED::Get ()) CActLED::Get ()->On ();
							int displayLen = (s_pThis && s_pThis->m_Cassette.get_len() > 0)
											 ? s_pThis->m_Cassette.get_len()
											 : tapeLen;
#ifdef CASSETTE_DEBUG_MESSAGE
							WriteLog("Cassette motor ON, tapePos=%d, tapeLen=%d\n", tapePos, displayLen);
#endif
							ScreenLog(13, "MOTOR ON, LEN:%d", displayLen);
						}
					else
					{
						if (CActLED::Get ()) CActLED::Get ()->Off ();
#ifdef CASSETTE_DEBUG_MESSAGE
						int mpos = (s_pThis && s_pThis->m_Cassette.get_len() > 0) ? s_pThis->m_Cassette.pos : tapePos;
						WriteLog("Cassette motor OFF, tapePos=%d (mpos=%d)\n", tapePos, mpos);
#endif
						ScreenLog(13, "MOTOR OFF");

						// Decode and display header info loaded at FILMOD (0x1396)
						byte mode = spcsys.RAM[0x1396];
						char name[17];
						int name_len = 0;
						for (int i = 0; i < 16; i++)
						{
							byte c = spcsys.RAM[0x1397 + i];
							if (c == 0x00) break;
							name[name_len++] = (c >= 0x20 && c <= 0x7E) ? c : '.';
						}
						name[name_len] = '\0';
						ScreenLog(11, "Hdr: Mode=0x%02X Name=%s", mode, name);
						// Show raw hex of FILMOD area
						ScreenLog(12, "H:%02X%02X%02X%02X %02X%02X%02X%02X %02X%02X",
							spcsys.RAM[0x1396], spcsys.RAM[0x1397],
							spcsys.RAM[0x1398], spcsys.RAM[0x1399],
							spcsys.RAM[0x139A], spcsys.RAM[0x139B],
							spcsys.RAM[0x139C], spcsys.RAM[0x139D],
							spcsys.RAM[0x139E], spcsys.RAM[0x139F]);
#ifdef CASSETTE_DEBUG_MESSAGE
					WriteLog("Loaded Header: Mode=0x%02X Name=%s\n", mode, name);
					WriteLog("Header RAM dump 0x1396-0x13B7:");
					for (int i = 0; i < 0x22; i++) WriteLog(" %02X", spcsys.RAM[0x1396 + i]);
					WriteLog("\n");
					WriteLog("BASIC text RAM dump 0x7C4E-0x7D4D:");
					for (int i = 0; i < 0x100; i++) WriteLog(" %02X", spcsys.RAM[0x7C4E + i]);
					WriteLog("\n");
					DumpScreen();
#endif
					}
				}
			}
		}
	}
	else if (Port == 0x4003)
	{
		if (Value == 0)
		{
			consecutiveZeros = 0;
			casLastTime = GetCycles();
			casReadVal = 0;
			spcsys.cas.button = 1; // CAS_PLAY
			spcsys.cas.motor = 1;
			int displayLen = (s_pThis && s_pThis->m_Cassette.get_len() > 0)
							 ? s_pThis->m_Cassette.get_len()
							 : tapeLen;
			ScreenLog(13, "MOTOR ON (via 4003), LEN:%d", displayLen);
		}
	}
	else if ((Port & 0xFFFE) == 0x4000) // PSG
	{
		if (s_pThis && s_pThis->m_pPSG)
			PSG_writeIO(s_pThis->m_pPSG, Port & 1, Value);
		if (!(Port & 0x01))
			spcsys.psgRegNum = Value & 0x1f;
	}
}

#ifndef HOST_COMPILE
int printf(const char *format, ...) { return 0; }
#endif

} // extern "C"
