#include "kernel.h"
#include <circle/timer.h>
#include <circle/bcmframebuffer.h>
#include <circle/util.h>

extern "C" {
#include "Z80.h"
#include "MC6847.h"
#include "spcall.h"
#include "common.h"
#include "spckey.h"
}

// 8bpp intermediate buffer (320x240) - MC6847 output
static u8 mc6847_buf[320*240];
// Palette: 8bpp index -> 16bpp RGB565
static u16 pal565[256];

// SPC-1000 system state
SPCSystem spcsys;
extern unsigned char ROM[32768];

// Key mapping
TKeyMap spcKeyHash[0x200];
unsigned char keyMatrix[10];
static CKernel *s_pThis;

// Stubs needed by MC6847.c
int bpp = 16;
char samsung_bmp_c[] = "";

#define RGB565(r,g,b) (((r)&0x1F)<<11 | ((g)&0x3F)<<5 | ((b)&0x1F))
#define I_PERIOD 4000
#define WAITTIME 983

CKernel::CKernel (void)
:	m_Memory (TRUE),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_Screen (320, 240),
	m_USBHCI (&m_Interrupt, &m_Timer, TRUE),
	m_pKeyboard (0)
{
	m_ActLED.Blink (5);
	s_pThis = this;
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	bOK = m_Screen.Initialize ();
	if (!bOK) return FALSE;

	if (bOK)
		bOK = m_Interrupt.Initialize ();

	if (bOK)
		bOK = m_Timer.Initialize ();

	if (bOK)
		bOK = m_USBHCI.Initialize ();

	if (bOK)
	{
		CDevice *pTarget = &m_Screen;
		bOK = m_Logger.Initialize (pTarget);
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
	pal565[8]  = RGB565(0xff>>3, 0x80>>2, 0x00>>3);   // ORANGE
	pal565[9]  = RGB565(0x3b>>3, 0x08>>2, 0xff>>3);   // CYANBLUE
	pal565[10] = RGB565(0x07>>3, 0xe3>>2, 0x99>>3);   // LGREEN
	pal565[11] = RGB565(0x00>>3, 0x00>>2, 0x00>>3);   // text bg CSS=0: BLACK
	pal565[12] = RGB565(0x07>>3, 0xff>>2, 0x00>>3);   // text fg CSS=0: GREEN
	pal565[13] = RGB565(0xcc>>3, 0x00>>2, 0x3b>>3);   // text bg CSS=1: RED
	pal565[14] = RGB565(0xff>>3, 0x80>>2, 0x00>>3);   // text fg CSS=1: ORANGE
	pal565[15] = RGB565(0xff>>3, 0xff>>2, 0x00>>3);   // YELLOW
	for (int i = 16; i < 256; i++)
		pal565[i] = pal565[i % 16];

	// Copy ROM
	memcpy(spcsys.ROM, ROM, 0x8000);

	// Reset SPC state
	spcsys.IPLK = 1;
	spcsys.GMODE = 0;
	memset(spcsys.VRAM, 0, 0x2000);
	memset(spcsys.RAM, 0, 0x10000);
	memset(keyMatrix, 0xff, 10);

	// Build key hash table
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
	unsigned ticks = m_Timer.GetClockTicks();

	while (1)
	{
		int count = R->ICount;
		ExecZ80(R);
		spcsys.cycles += (count - R->ICount);

		if (R->ICount <= 0)
		{
			frame++;
			spcsys.tick++;
			R->ICount += I_PERIOD;

			// Poll USB plug-and-play (~1/sec)
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

			// Z80 interrupt every 16 frames
			if (frame % 16 == 0)
			{
				if (R->IFF & IFF_EI)
				{
					R->IFF |= IFF_IM1 | IFF_1;
					IntZ80(R, 0);
				}
			}

			// Update screen every 33 frames
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

			// Frame timing
			unsigned elapsed = m_Timer.GetClockTicks() - ticks;
			if (elapsed < WAITTIME)
				m_Timer.usDelay(WAITTIME - elapsed);
			ticks = m_Timer.GetClockTicks();
		}
	}

	return ShutdownHalt;
}

void CKernel::KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6])
{
	// Reset key matrix
	memset(keyMatrix, 0xff, 10);

	// Map modifier keys
	for (int i = 0; spcKeyMap[i].keyMatIdx != -1; i++)
	{
		if (spcKeyMap[i].sym & 0x100)
		{
			unsigned char mod_bit = spcKeyMap[i].sym & 0xFF;
			if (ucModifiers & mod_bit)
				keyMatrix[spcKeyMap[i].keyMatIdx] &= ~spcKeyMap[i].keyMask;
		}
	}

	// Map raw keys
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
}

int printf(const char *format, ...) { return 0; }

} // extern "C"
