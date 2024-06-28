//
// SpectrumScreen.cpp
//
// Spectrum screen emulator code provided by Jose Luis Sanchez
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
#include "MC6847Screen.h"
#include <circle/util.h>
#include <assert.h>



CMC6847Screen::CMC6847Screen (void)
:	m_pFrameBuffer (0),
	m_pBuffer (0),
	m_pVideoMem (0)
{
}

CMC6847Screen::~CMC6847Screen (void)
{
	m_pVideoMem = 0;
	m_pBuffer = 0;

	delete m_pFrameBuffer;
	m_pFrameBuffer = 0;
}

boolean CMC6847Screen::Initialize (u8 *pVideoMem)
{
	m_pVideoMem = pVideoMem;
	assert (m_pVideoMem != 0);

	m_pFrameBuffer = new CBcmFrameBuffer (320, 240, 4);
	assert (m_pFrameBuffer != 0);

	// Color palette definition, uses RGB565 format
	// Needs to be defined BEFORE call Initialize
	m_pFrameBuffer->SetPalette ( 0, COLOR16(0x00, 0x00, 0x00));  // black
	m_pFrameBuffer->SetPalette ( 1, COLOR16(0x07, 0xff, 0x00));  // green
	m_pFrameBuffer->SetPalette ( 2, COLOR16(0xff, 0xff, 0x00));  // yellow
	m_pFrameBuffer->SetPalette ( 3, COLOR16(0x3b, 0x08, 0xff));  // blue
	m_pFrameBuffer->SetPalette ( 4, COLOR16(0xcc, 0x00, 0x3b));  // red
	m_pFrameBuffer->SetPalette ( 5, COLOR16(0xff, 0xff, 0xff));  // buff
	m_pFrameBuffer->SetPalette ( 6, COLOR16(0x07, 0xe3, 0x99));  // cyan
	m_pFrameBuffer->SetPalette ( 7, COLOR16(0xff, 0x1c, 0xff));  // magenta
	m_pFrameBuffer->SetPalette ( 8, COLOR16(0xff, 0x81, 0x00));  // orange
	m_pFrameBuffer->SetPalette ( 9, COLOR16(0x07, 0xff, 0x00));  // green
	m_pFrameBuffer->SetPalette (10, COLOR16(0xff, 0xff, 0xff)); // buff
	m_pFrameBuffer->SetPalette (11, COLOR16(0x00, 0x3f, 0x00)); // dark green
	m_pFrameBuffer->SetPalette (12, COLOR16(0x07, 0xff, 0x00)); // bright green
	m_pFrameBuffer->SetPalette (13, COLOR16(0x91, 0x00, 0x00)); // dark orange
	m_pFrameBuffer->SetPalette (14, COLOR16(0xff, 0x81, 0x00)); // bright orange
	m_pFrameBuffer->SetPalette (0xff, COLOR16(0xff, 0xff, 0xff));
	m_pFrameBuffer->SetPalette (0x46, COLOR16(0xff, 0x00, 0x00)); 
	if (!m_pFrameBuffer->Initialize()) {
		return FALSE;
	}

	m_pBuffer = (u32 *) (uintptr) m_pFrameBuffer->GetBuffer();
	assert (m_pBuffer != 0);

	memset(m_pBuffer, 0xFF, m_pFrameBuffer->GetSize());

	// Create a lookup table for draw the screen Faster Than Light :)
	// for (int attr = 0; attr < 128; attr++) {
	// 	unsigned ink = attr & 0x07;
	// 	unsigned paper = (attr & 0x78) >> 3;
	// 	if (attr & 0x40) {
	// 		ink |= 0x08;
	// 	}
	// 	for (int idx = 0; idx < 256; idx++) {
	// 		for (unsigned mask = 0x80; mask > 0; mask >>= 1) {
	// 			m_scrTable[attr][idx] <<= 4;
	// 			m_scrTable[attr + 128][idx] <<= 4;
	// 			if (idx & mask) {
	// 				m_scrTable[attr][idx] |= ink;
	// 				m_scrTable[attr + 128][idx] |= paper;
	// 			} else {
	// 				m_scrTable[attr][idx] |= paper;
	// 				m_scrTable[attr + 128][idx] |= ink;
	// 			}
	// 		}
	// 		m_scrTable[attr][idx] = bswap32(m_scrTable[attr][idx]);
	// 		m_scrTable[attr + 128][idx] = bswap32(m_scrTable[attr + 128][idx]);
	// 	}
	// }

	return TRUE;
}

#define GMODE 0
#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 320
#define VDP_HEIGHT 192
#define VDP_WIDTH 256
#define _BIT(a, b) (a & (1 << b))
#define SCREEN_TEXT_START 0
#define SCREEN_ATTR_START 0x800
#define FILL(a,d,c) pos=d;while(pos-->0)*a++=c
typedef unsigned char PIXEL;

void CMC6847Screen::Update (boolean flash)
{
	assert (m_pBuffer != 0);
	assert (m_pVideoMem != 0);

	int pos = 0;
	u8 gmode = GMODE;
	u8 *data = (u8 *)m_pBuffer;
	u8 *VRAM = m_pVideoMem;
	u8 _gm0, _gm1, _ag, _css;
	u16 _page, y, h, x, mask;
	u8 attr, ch, b, cix, c;
	_gm0 = _BIT(gmode, 2);
	_gm1 = _BIT(gmode, 1);
	//int _gm2 = 1;
	_ag = _BIT(gmode, 3);
	_css = _BIT(gmode, 7);
	_page = gmode >> 4 & 0x3;
	PIXEL bg, fg, border;
	rept = (SCREEN_HEIGHT - VDP_HEIGHT)/2 * SCREEN_WIDTH;
	repl = (SCREEN_WIDTH - VDP_WIDTH)/2;
	repr = (SCREEN_WIDTH - VDP_WIDTH)/2;
	repb = (SCREEN_HEIGHT - VDP_HEIGHT)/2 * SCREEN_WIDTH;
	border = cMap[0];
	b = 0;
	if (_ag == 0)
    {
		FILL(data, rept, border);
		for(y=0; y < 16; y++)
		{
			for(h=0; h < 12; h++)
			{
			FILL(data, repl, border);
			for(x=0; x < 32; x++)
			{
				attr = VRAM[x + y * 32 + SCREEN_ATTR_START + _page * 0x200];
				ch = VRAM[x + y * 32 + SCREEN_TEXT_START + _page * 0x200];
				if ((attr & ATTR_SEM) != 0)
				{
					bg = cMap[0];
					if ((attr & ATTR_EXT) != 0)
					{
						fg = cMap[(((attr & ATTR_CSS) << 1) | ((ch & 0xc0) >> 6)) + 1];
						b = semiGrFont1[(ch & 0x3f) * 12 + h];	
					} 
					else 
					{
						fg = cMap[((ch & 0x70)>> 4) + 1];
						//printf("fg=%d,%d\n", ch, ((ch & 0x70)>> 4) + 1);
						b = semiGrFont0[(ch & 0x0f) * 12 + h];
					}
				}
				else // ASCII
				{
					cix = (attr & ATTR_CSS) >> 1; 
					if ((attr & ATTR_INV) == 0)
					{
						bg = cMap[11 + cix * 2];
						fg = cMap[11 + cix * 2 + 1];
					}
					else
					{
						fg = cMap[11 + cix * 2];
						bg = cMap[11 + cix * 2 + 1];
					}
					if (ch < 32 && ((attr & ATTR_EXT) == 0))
						ch = 32;	
					if (((attr & ATTR_EXT) != 0) && (ch < 96))
						ch += 128;
					if (ch >= 96 && ch < 128)
						b = VRAM[0x1600+(ch-96)*16+h];
					else if (ch >= 128 && ch < 224)
						b = VRAM[0x1000+(ch-128)*16+h];
					else if (ch >= 32)
						b = CGROM[(ch-32)*12+h];
				}
				for(mask = 0x80; mask != 0; mask >>=1)
				{
					*data++ = (((b & mask) != 0) ? fg : bg);
				}
			}
			FILL(data, repr, border);
			}
		}
		FILL(data, repb, border);
    }
    else
    {
		bg = cMap[0];
		border = fg = (_css ? cMap[10] : cMap[9]);
		//border = (_css ? cMap[10] : cMap[9]);
		FILL(data, rept, border);
		for(y = 0; y < 192; y++)
		{
			FILL(data, repl, border);
			for(x = 0; x < 32; x++)
			{
				b = VRAM[y * 32 + x];
				if (_gm1)
				{
					if (_gm0)
					{ 
						for(mask = 0x80; mask != 0; mask >>=1)
						{	
							*data++ = (b & mask) != 0 ? fg : bg;
						}
					}
					else // _gm0 == 0
					{
						for(c = 6; c < 8 && c >= 0; c-=2)
						{
							*data++ = cMap[((b & (0x3 << c)) >> c) + (_css ? 5 : 1)];
							*data++ = cMap[((b & (0x3 << c)) >> c) + (_css ? 5 : 1)];
						}
					}
				}
				else
				{
					if (_gm0)
					{
						for(mask = 0x80; mask != 0; mask >>=1)
						{	
							*data++ = (b & mask) != 0 ? fg : bg;
							*data++ = (b & mask) != 0 ? fg : bg;
						}
					}
					else
					{
						for(c = 6; c > 0; c-=2)
						{
							*(data + 256 + 1) = *(data + 256) = *(data+1) = *data = cMap[((b & (0x3 << c)) >> c) + (_css ? 5 : 1)];
							data+=2;
						}
					}
				} 
			}
			FILL(data, repr, border);
	    }
      	FILL(data, repb, border);
    }
}

