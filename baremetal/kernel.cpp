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
static int cksm_check_count = 0;
static unsigned short last_calculated_cksm = 0;
static unsigned short last_stored_cksm = 0;
static bool cksm_updated = false;

static void WriteLog(const char *format, ...)
{
    FIL File;
    if (f_open (&File, "SD:/log.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK)
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
}

static void ScreenLog(int row, const char *format, ...)
{
    char buf[33];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    
#ifdef HOST_COMPILE
    printf("[ScreenLog Row %02d] %s\n", row, buf);
    fflush(stdout);
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

// Apply ROM checksum bypass patches according to config
static void ApplyChecksumBypass(void)
{
    if (!tapeCfg.checksum_bypass_enabled)
        return;

    for (int i = 0; i < tapeCfg.checksum_patch_count && i < MAX_EXTRA_PATCHES; i++)
    {
        unsigned short addr = tapeCfg.checksum_patch_addr[i];
        if (addr < 0x8000)
        {
            spcsys.ROM[addr] = ROM[addr] = tapeCfg.checksum_patch_value[i];
        }
    }
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
static unsigned int casBitEndTime = 0;
static unsigned int casBitInvTime = 0;

// ReadTapeBit - reads a single bit from the tape bit-stream sequentially.
//
// The SPC-1000 BIOS polls I/O port 0x4001 (bit 7) once per EDGE call with no
// timing delay (WAITR is a NOP), so each poll must consume exactly one tape
// bit. The BIOS' own MKRD/CLOAD0/VBLOAD routines perform all pilot/sync
// detection, byte framing (8 data bits MSB-first + 1 stop bit) and checksum
// verification natively. The emulator therefore only needs to feed bits in
// order - no pattern scanning, no offset shifting and no byte injection.
//
// zero_skip accelerates turbo-loading by fast-forwarding through long runs
// of zero bits (inter-block silence gaps). It only fires once more than
// `zero_skip` consecutive zeros have been seen, which never happens inside
// the MKRD zero-count phase (max 40 zeros) or inside the data payload (max 8
// consecutive zero bits), so it cannot disturb bit alignment.
static int ReadTapeBit(void)
{
    if (tapeLen == 0)
    {
        int len = 0;
        while (tap0[len]) len++;
        tapeLen = len;
    }

    if (tapePos >= tapeLen)
    {
        return 0;
    }

    int c;
    int zero_skip = tapeCfg.zero_skip;
    if (zero_skip < 1) zero_skip = 1;

    if (consecutiveZeros > zero_skip)
    {
        // Skip the remainder of a long zero gap, then return the first '1'.
        while (tapePos < tapeLen && tap0[tapePos] == '0')
            tapePos++;
        if (tapePos < tapeLen)
            c = (tap0[tapePos++] == '1' ? 1 : 0);
        else
            c = 0;
        consecutiveZeros = 0;
    }
    else
    {
        c = (tap0[tapePos++] == '1' ? 1 : 0);
        if (c == 0)
            consecutiveZeros++;
        else
            consecutiveZeros = 0;
    }

    return c;
}

// CasRead - returns the next FSK-demodulated bit for I/O port 0x4001 (PSG
// register 14, bit 7). Feeding one bit per poll lets the SPC-1000 BIOS load
// the tape cleanly with correct checksums, eliminating the previous
// alignment hacks (MKRD return-address sniffing, +10 offset, byte injection).
static int CasRead(void)
{
    return ReadTapeBit();
}

TKeyMap spcKeyHash[0x200];
unsigned char keyMatrix[10] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
volatile bool g_reset_requested = false;
volatile bool g_ipl_reset = false;
static CKernel *s_pThis;

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
	// Load tape loader configuration and apply ROM checksum bypass patches
	LoadTapeConfig();
	ApplyChecksumBypass();
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

TShutdownMode CKernel::Run (void)
{
	InitMC6847(mc6847_buf, spcsys.VRAM, 256, 192);

	CBcmFrameBuffer *pFB = m_Screen.GetFrameBuffer();
	u16 *pScreen = (u16 *)(uintptr)pFB->GetBuffer();
	unsigned sw = pFB->GetWidth();
	unsigned sh = pFB->GetHeight();
	unsigned pitch = pFB->GetPitch() / 2;
	unsigned offX = (sw > 320) ? (sw - 320) / 2 : 0;
	unsigned offY = (sh > 240) ? (sh - 240) / 2 : 0;

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
			// Patch ROM to bypass checksum verification according to config
			ApplyChecksumBypass();
			spcsys.IPLK = 1;
			ResetZ80(R);
			memset(keyMatrix, 0xff, 10);
			tapePos = 0;
			consecutiveZeros = 0;
			casReadVal = 0;
			TapeLoaderConfig_ResetInjections(&tapeCfg);
			cksm_check_count = 0;
			last_calculated_cksm = 0;
			last_stored_cksm = 0;
			cksm_updated = false;
			spcsys.cas.motor = 0;
			spcsys.cas.pulse = 0;
			spcsys.cycles = 0;
			ScreenLog(13, "RESET SYSTEM (%s)", g_ipl_reset ? "IPL" : "NORMAL");
		}

		if (R->PC.W == 0x0189)
		{
			unsigned short calculated = R->HL.W;
			unsigned short stored = (R->DE.B.l << 8) | R->AF.B.h;
			last_calculated_cksm = calculated;
			last_stored_cksm = stored;
			cksm_updated = true;
			ScreenLog(14, "Cksm %d Cal:%04X Std:%04X", cksm_check_count++, calculated, stored);
			WriteLog("Checksum check %d: Calc=%04X, Stored=%04X\n", cksm_check_count - 1, calculated, stored);
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

			// Sleep only if motor is off, to allow turbo load when motor is on
			if (!spcsys.cas.motor)
			{
				m_Scheduler.MsSleep(1);
			}
			else if (frame % 128 == 0)
			{
				m_Scheduler.Yield();
			}
		}
	}

	return ShutdownHalt;
}

void CKernel::KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6])
{
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
		return keyMatrix[Port - 0x8000];
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
					if (CasRead() == 1) r |= 0x80;
					else r &= 0x7f;
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
		return (tapeLen > 0 ? 1 : 0);
	}
	else if (Port == 0x4004)
	{
		byte retval = 0;
		for (int i = 0; i < 8; i++)
		{
			if (ReadTapeBit())
				retval |= (1 << (7 - i));
		}
		ReadTapeBit(); // Skip stop bit/extra bit
		return retval;
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
					if (spcsys.cas.motor)
					{
						casLastTime = GetCycles();
						consecutiveZeros = 0;
						casReadVal = 0;
						if (CActLED::Get ()) CActLED::Get ()->On ();
						WriteLog("Cassette motor ON, tapeLen=%d\n", tapeLen);
						ScreenLog(13, "MOTOR ON, LEN:%d", tapeLen);
					}
					else
					{
						if (CActLED::Get ()) CActLED::Get ()->Off ();
						WriteLog("Cassette motor OFF\n");
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
						WriteLog("Loaded Header: Mode=0x%02X Name=%s\n", mode, name);
						if (cksm_updated)
						{
							ScreenLog(14, "Cksm Cal:%04X Std:%04X", last_calculated_cksm, last_stored_cksm);
							WriteLog("Last Checksum: Calc=%04X, Stored=%04X\n", last_calculated_cksm, last_stored_cksm);
						}
					}
				}
			}
		}
	}
	else if (Port == 0x4003)
	{
		if (Value == 0)
		{
			tapePos = 0;
			consecutiveZeros = 0;
			casLastTime = GetCycles();
			casReadVal = 0;
			spcsys.cas.button = 1; // CAS_PLAY
			spcsys.cas.motor = 1;
			ScreenLog(13, "MOTOR ON (via 4003), LEN:%d", tapeLen);
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
