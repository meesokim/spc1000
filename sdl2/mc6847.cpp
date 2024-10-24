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
#include "mc6847.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>


CMC6847::CMC6847 (void)
:	m_pBuffer (0)
{
	GMODE = 0;
}

CMC6847::~CMC6847 (void)
{
	delete m_pBuffer;
}

// really ((green) & 0x3F) << 5, but to have a 0-31 range for all colors
#define COLOR16(red, green, blue)	  (((red>>3) & 0x1F) << 11 \
					| ((green>>2) & 0x3F) << 6 \
					| ((blue>>3) & 0x1F))

// BGRA (was RGBA with older firmware)
#define COLOR32(red, green, blue, alpha)  (((red) & 0xFF)       \
					| ((green) & 0xFF) << 8  \
					| ((blue) & 0xFF)   << 16 \
					| ((alpha) & 0xFF) << 24)

#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 320
#define VDP_HEIGHT 192
#define VDP_WIDTH 256
#define _BIT(a, b) (a & (1 << b))
#define SCREEN_TEXT_START 0
#define SCREEN_ATTR_START 0x800
#define FILL(a,d,c) pos=d;while(pos-->0)*a++=palette[c]
#define REPT (SCREEN_HEIGHT - VDP_HEIGHT)/2 * SCREEN_WIDTH
#define REPL (SCREEN_WIDTH - VDP_WIDTH)/2
#define REPR (SCREEN_WIDTH - VDP_WIDTH)/2
#define REPB (SCREEN_HEIGHT - VDP_HEIGHT)/2 * SCREEN_WIDTH
#define FBSIZE (SCREEN_HEIGHT * SCREEN_WIDTH)
typedef unsigned char PIXEL;

bool CMC6847::Initialize ()
{
	memset(VRAM, 0, sizeof(VRAM));
	GMODE = 0;
	m_pBuffer = new u16[FBSIZE];
	m_pBuffer0 = new u16[FBSIZE];
	assert (m_pBuffer != 0);

	// Color palette definition, uses RGB565 format
	// Needs to be defined BEFORE call Initialize

	palette[0]    = COLOR16(0x00, 0x00, 0x00);  // black
	palette[1]    = COLOR16(0x07, 0xff, 0x00);  // green
	palette[2]    = COLOR16(0xff, 0xff, 0x00);  // yellow
	palette[3]    = COLOR16(0x3b, 0x08, 0xff);  // blue
	palette[4]    = COLOR16(0xcc, 0x00, 0x3b);  // red

	palette[5]    = COLOR16(0xff, 0xff, 0xff);  // buff
	palette[6]    = COLOR16(0x07, 0xe3, 0x99);  // cyan
	palette[7]    = COLOR16(0xff, 0x1c, 0xff);  // magenta
	palette[8]    = COLOR16(0xff, 0x81, 0x00);  // orange
	palette[9]    = COLOR16(0x07, 0xff, 0x00);  // green

	palette[10]   = COLOR16(0xff, 0xff, 0xff); // buff

	palette[11]   = COLOR16(0x00, 0x3f, 0x00); // dark green
	palette[12]   = COLOR16(0x07, 0xff, 0x00); // bright green
	palette[13]   = COLOR16(0x91, 0x00, 0x00); // dark orange
	palette[14]   = COLOR16(0xff, 0x81, 0x00); // bright orange
	palette[0xff] = COLOR16(0xff, 0xff, 0xff);
	palette[0x46] = COLOR16(0xff, 0x00, 0x00); 

	// memset(m_pVideoMem, 0x0, SCREEN_HEIGHT * SCREEN_WIDTH);

    int i, j;
	for (i = 0; i < 18; i++)
		cMap[i] = i;
	for (i = 0; i < 16; i++)
    {
		for(j = 0; j < 12; j++)
		{
			unsigned char val = 0;
			if (j < 6)
			{
				if (i & 0x08) val |= 0xf0;
				if (i & 0x04) val |= 0x0f;
			}
			else
			{
				if (i & 0x02) val |= 0xf0;
				if (i & 0x01) val |= 0x0f;
			}
			semiGrFont0[i*12+j] = val;
		}
    }
	for (i = 0; i < 64; i++)
    {
		for(j = 0; j < 12; j++)
		{
			unsigned char val = 0;
			if (j < 4)
			{
				if (i & 0x20) val |= 0xf0;
				if (i & 0x10) val |= 0x0f;
			}
			else if (j < 8)
			{
				if (i & 0x08) val |= 0xf0;
				if (i & 0x04) val |= 0x0f;
			}
			else
			{
				if (i & 0x02) val |= 0xf0;
				if (i & 0x01) val |= 0x0f;
			}
			semiGrFont1[i*12+j] = val;
		}
    }
	return true;
}

void CMC6847::Update ()
{
	assert (m_pBuffer != 0);

	int pos = 0;
	u8 gmode = GMODE;
	u16 *data = (u16 *)m_pBuffer;
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
	border = cMap[0];
	b = 0;
	if (_ag == 0)
    {
		FILL(data, REPT, border);
		for(y=0; y < 16; y++)
		{
			for(h=0; h < 12; h++)
			{
			FILL(data, REPL, border);
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
					*data++ = palette[(((b & mask) != 0) ? fg : bg)];
				}
			}
			FILL(data, REPR, border);
			}
		}
		FILL(data, REPB, border);
    }
    else
    {
		bg = cMap[0];
		border = fg = (_css ? cMap[10] : cMap[9]);
		//border = (_css ? cMap[10] : cMap[9]);
		FILL(data, REPT, border);
		for(y = 0; y < 192; y++)
		{
			FILL(data, REPL, border);
			for(x = 0; x < 32; x++)
			{
				b = VRAM[y * 32 + x];
				if (_gm1)
				{
					if (_gm0)
					{ 
						for(mask = 0x80; mask != 0; mask >>=1)
						{	
							*data++ = palette[(b & mask) != 0 ? fg : bg];
						}
					}
					else // _gm0 == 0
					{
						for(c = 6; c < 8 && c >= 0; c-=2)
						{
							*data++ = palette[cMap[((b & (0x3 << c)) >> c) + (_css ? 5 : 1)]];
							*data++ = palette[cMap[((b & (0x3 << c)) >> c) + (_css ? 5 : 1)]];
						}
					}
				}
				else
				{
					if (_gm0)
					{
						for(mask = 0x80; mask != 0; mask >>=1)
						{	
							*data++ = palette[(b & mask) != 0 ? fg : bg];
							*data++ = palette[(b & mask) != 0 ? fg : bg];
						}
					}
					else
					{
						for(c = 6; c > 0; c-=2)
						{
							*(data + 256 + 1) = *(data + 256) = *(data+1) = *data = palette[cMap[((b & (0x3 << c)) >> c) + (_css ? 5 : 1)]];
							data+=2;
						}
					}
				} 
			}
			FILL(data, REPR, border);
	    }
      	FILL(data, REPB, border);
    }
	memcpy(m_pBuffer0, m_pBuffer, FBSIZE*2);
}

u16 * CMC6847::GetBuffer() {
	return m_pBuffer0;
}

/**
 * MC6847 Character Generator Internal ROM Data. (provided by Zanny)
 */
unsigned char CMC6847::CGROM[] =
{
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //32
 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x08, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x14, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x14, 0x14, 0x3E, 0x14, 0x3E, 0x14, 0x14, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x08, 0x1E, 0x28, 0x1C, 0x0A, 0x3C, 0x08, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x32, 0x32, 0x04, 0x08, 0x10, 0x26, 0x26, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x10, 0x28, 0x10, 0x28, 0x26, 0x24, 0x1A, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x04, 0x08, 0x10, 0x10, 0x10, 0x08, 0x04, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x10, 0x08, 0x04, 0x04, 0x04, 0x08, 0x10, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x08, 0x1C, 0x3E, 0x1C, 0x08, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x08, 0x10, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x02, 0x02, 0x04, 0x08, 0x10, 0x20, 0x20, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x0C, 0x12, 0x12, 0x12, 0x12, 0x12, 0x0C, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x08, 0x18, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x1C, 0x22, 0x02, 0x1C, 0x20, 0x20, 0x3E, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x1C, 0x22, 0x02, 0x0C, 0x02, 0x22, 0x1C, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x04, 0x0C, 0x14, 0x24, 0x3E, 0x04, 0x04, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x3E, 0x20, 0x3C, 0x02, 0x02, 0x22, 0x1C, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x1C, 0x22, 0x20, 0x3C, 0x22, 0x22, 0x1C, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x3E, 0x02, 0x02, 0x04, 0x08, 0x08, 0x08, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x1C, 0x22, 0x22, 0x1C, 0x22, 0x22, 0x1C, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x1C, 0x22, 0x22, 0x1E, 0x02, 0x22, 0x1C, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x08, 0x10, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x1C, 0x22, 0x02, 0x04, 0x08, 0x00, 0x08, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x1C, 0x22, 0x22, 0x06, 0x0A, 0x0A, 0x06, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x08, 0x14, 0x22, 0x3E, 0x22, 0x22, 0x22, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x3C, 0x22, 0x22, 0x3C, 0x22, 0x22, 0x3C, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x1C, 0x22, 0x20, 0x20, 0x20, 0x22, 0x1C, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x3C, 0x12, 0x12, 0x12, 0x12, 0x12, 0x3C, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x3E, 0x20, 0x20, 0x3C, 0x20, 0x20, 0x3E, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x3E, 0x20, 0x20, 0x3C, 0x20, 0x20, 0x20, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x1C, 0x22, 0x20, 0x26, 0x22, 0x22, 0x1E, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x22, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x1C, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x0E, 0x04, 0x04, 0x04, 0x04, 0x24, 0x18, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x22, 0x24, 0x28, 0x30, 0x28, 0x24, 0x22, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3E, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x22, 0x36, 0x2A, 0x2A, 0x22, 0x22, 0x22, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x22, 0x32, 0x2A, 0x26, 0x22, 0x22, 0x22, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x3E, 0x22, 0x22, 0x22, 0x22, 0x22, 0x3E, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x3C, 0x22, 0x22, 0x3C, 0x20, 0x20, 0x20, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x1C, 0x22, 0x22, 0x22, 0x2A, 0x26, 0x1E, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x3C, 0x22, 0x22, 0x3C, 0x28, 0x24, 0x22, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x1C, 0x22, 0x20, 0x1C, 0x02, 0x22, 0x1C, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x3E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1C, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x2A, 0x2A, 0x36, 0x22, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x22, 0x22, 0x14, 0x08, 0x14, 0x22, 0x22, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x22, 0x22, 0x14, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x3E, 0x02, 0x04, 0x08, 0x10, 0x20, 0x3E, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x1C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1C, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x08, 0x14, 0x22, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x10, 0x20, 0x7F, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00,

 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //64
 0x00, 0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x3C, 0x7E, 0x7E, 0x7E, 0x7E, 0x3C, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x2A, 0x1C, 0x08, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x08, 0x04, 0x7E, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
 0x10, 0x10, 0x10, 0x10, 0x10, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x10, 0x10, 0x10, 0x10, 0x10, 0xFF, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x10, 0x10, 0x10, 0x10, 0x10, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
 0x10, 0x10, 0x10, 0x10, 0x10, 0xF0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
 0x00, 0x72, 0x8A, 0x8A, 0x72, 0x02, 0x3E, 0x02, 0x3E, 0x20, 0x3E, 0x00,
 0x00, 0x72, 0x8A, 0x72, 0xFA, 0x2E, 0x42, 0x3E, 0x3E, 0x20, 0x3E, 0x00,
 0x00, 0x22, 0x22, 0xFA, 0x02, 0x72, 0x8B, 0x8A, 0x72, 0x22, 0xFA, 0x00,
 0x00, 0x10, 0x10, 0x28, 0x44, 0x82, 0x00, 0xFE, 0x10, 0x10, 0x10, 0x10,
 0x00, 0x7C, 0x44, 0x7C, 0x10, 0xFE, 0x00, 0x7C, 0x04, 0x04, 0x04, 0x00,
 0x00, 0x7C, 0x04, 0x04, 0x00, 0xFE, 0x00, 0x7C, 0x44, 0x44, 0x7C, 0x00,
 0x00, 0x7C, 0x40, 0x78, 0x40, 0x40, 0x7C, 0x10, 0x10, 0x10, 0xFE, 0x00,
 0x00, 0x82, 0x8E, 0x82, 0x8E, 0x82, 0xFA, 0x02, 0x40, 0x40, 0x7E, 0x00,
 0x00, 0x02, 0x22, 0x22, 0x22, 0x52, 0x52, 0x8A, 0x8A, 0x02, 0x02, 0x00,
 0x00, 0x44, 0x7C, 0x44, 0x7C, 0x00, 0xFE, 0x10, 0x50, 0x40, 0x7C, 0x00,
 0x00, 0x10, 0x10, 0xFE, 0x28, 0x44, 0x82, 0x10, 0x10, 0x10, 0xFE, 0x00,
 0x00, 0x01, 0x05, 0xF5, 0x15, 0x15, 0x17, 0x25, 0x45, 0x85, 0x05, 0x00,
 0x00, 0x01, 0x05, 0xF5, 0x85, 0x85, 0x87, 0x85, 0xF5, 0x05, 0x05, 0x00,
 0x00, 0x02, 0x72, 0x8A, 0x8A, 0x8A, 0x72, 0x02, 0x42, 0x40, 0x7E, 0x00,
 0x00, 0x00, 0x7C, 0x40, 0x40, 0x40, 0x7C, 0x10, 0x10, 0x10, 0xFE, 0x00,
 0x00, 0x02, 0x72, 0x8A, 0x72, 0xFA, 0x2E, 0x42, 0x22, 0x20, 0x3E, 0x00,

 0x00, 0x00, 0x00, 0x3E, 0x22, 0x3E, 0x22, 0x3E, 0x00, 0x00, 0x00, 0x00, // 128
 0x00, 0x00, 0x3E, 0x22, 0x3E, 0x22, 0x3E, 0x22, 0x42, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x10, 0x54, 0x54, 0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x10, 0x12, 0xFC, 0x38, 0x34, 0x52, 0x91, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x10, 0x10, 0xFE, 0x10, 0x38, 0x54, 0x92, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x10, 0x28, 0x7C, 0x92, 0x7C, 0x54, 0xFE, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x10, 0x10, 0x10, 0x7C, 0x10, 0x10, 0xFE, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x20, 0x7E, 0x80, 0x7C, 0x50, 0xFE, 0x10, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x08, 0xFC, 0xA8, 0xFE, 0xA4, 0xFE, 0x14, 0x04, 0x00, 0x00,
 0x00, 0x00, 0x10, 0x28, 0x44, 0xFE, 0x14, 0x24, 0x48, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x74, 0x24, 0xF5, 0x65, 0xB2, 0xA4, 0x28, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x10, 0x10, 0x10, 0x54, 0x92, 0x30, 0x10, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x10, 0x10, 0xFE, 0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x10, 0x28, 0x7C, 0x82, 0x7C, 0x44, 0x7C, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x3C, 0x44, 0xA8, 0x10, 0x3E, 0xE2, 0x3E, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x08, 0x7F, 0x08, 0x7F, 0x08, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x06, 0x18, 0x20, 0x18, 0x06, 0x00, 0x3E, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x30, 0x0C, 0x02, 0x0C, 0x30, 0x00, 0x3E, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00, 0x3E, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x22, 0x14, 0x08, 0x14, 0x22, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x3E, 0x49, 0x7F, 0x49, 0x3E, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x08, 0x14, 0x22, 0x7F, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x7F, 0x22, 0x14, 0x08, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x08, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
 0x0C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x60,
 0x00, 0x0F, 0x08, 0x08, 0x08, 0x48, 0xA8, 0x18, 0x08, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x7E, 0x20, 0x10, 0x20, 0x7E, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x3E, 0x14, 0x14, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00,

 0x00, 0x60, 0x90, 0x90, 0x90, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 160
 0x00, 0x20, 0x60, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x60, 0x90, 0x20, 0x40, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x60, 0x90, 0x20, 0x90, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x60, 0xA0, 0xA0, 0xF0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0xF0, 0x80, 0xF0, 0x10, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x60, 0x80, 0xF0, 0x90, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0xF0, 0x10, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x60, 0x90, 0x60, 0x90, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x60, 0x90, 0xF0, 0x10, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x02, 0x34, 0x48, 0x48, 0x36, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x18, 0x24, 0x38, 0x24, 0x24, 0x38, 0x20, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x4E, 0x30, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x18, 0x24, 0x24, 0x3C, 0x24, 0x24, 0x18, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x1C, 0x20, 0x20, 0x18, 0x24, 0x24, 0x18, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x08, 0x1C, 0x2A, 0x2A, 0x1C, 0x08, 0x00, 0x00, 0x00,
 0x80, 0x40, 0x40, 0x20, 0x10, 0x10, 0x08, 0x04, 0x04, 0x02, 0x01, 0x01,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x7C, 0x00, 0xFE, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0xFE, 0xAA, 0xAA, 0xAA, 0xFE, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x7C, 0x10, 0x7C, 0x14, 0x14, 0xFE, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x10, 0x10, 0xFE, 0x00, 0x28, 0x44, 0x82, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x20, 0x20, 0x20, 0xFE, 0x20, 0x20, 0x3E, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x44, 0x82, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x20, 0x20, 0xFC, 0x24, 0x24, 0x44, 0x86, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x36, 0x49, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x18, 0x20, 0x18, 0x24, 0x18, 0x04, 0x18, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x08, 0x22, 0x14, 0x49, 0x14, 0x22, 0x08, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x38, 0x00, 0x7C, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00,
 0x00, 0x60, 0x90, 0x6E, 0x11, 0x10, 0x10, 0x11, 0x0E, 0x00, 0x00, 0x00,
 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10, 0x20, 0x20, 0x40, 0x80, 0x80,

 0x00, 0x00, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 192
 0x00, 0x00, 0x00, 0x00, 0x3C, 0x02, 0x1E, 0x22, 0x1F, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x20, 0x20, 0x2C, 0x32, 0x22, 0x32, 0x2C, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x1C, 0x22, 0x20, 0x22, 0x1C, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x02, 0x02, 0x1A, 0x26, 0x22, 0x26, 0x1A, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x1C, 0x22, 0x3E, 0x20, 0x1E, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x0C, 0x10, 0x10, 0x7C, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x1A, 0x26, 0x22, 0x26, 0x1A, 0x02, 0x1C, 0x00,
 0x00, 0x00, 0x20, 0x20, 0x2C, 0x32, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x08, 0x00, 0x18, 0x08, 0x08, 0x08, 0x1C, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x04, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x24, 0x18, 0x00, 0x00,
 0x00, 0x00, 0x20, 0x20, 0x22, 0x24, 0x28, 0x34, 0x22, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x18, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x76, 0x49, 0x49, 0x49, 0x49, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x2C, 0x32, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x1C, 0x22, 0x22, 0x22, 0x1C, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x2C, 0x32, 0x22, 0x32, 0x2C, 0x20, 0x20, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x1A, 0x26, 0x22, 0x26, 0x1A, 0x02, 0x02, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x2E, 0x30, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x1E, 0x20, 0x1C, 0x02, 0x3C, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x10, 0x10, 0x38, 0x10, 0x10, 0x12, 0x0C, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x26, 0x1A, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x14, 0x08, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x49, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x22, 0x14, 0x08, 0x14, 0x22, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x26, 0x1A, 0x02, 0x1C, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x3E, 0x04, 0x08, 0x10, 0x3E, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x08, 0x10, 0x10, 0x20, 0x10, 0x10, 0x08, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x54, 0xFE, 0x54, 0xFE, 0x54, 0x28, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x10, 0x08, 0x08, 0x04, 0x08, 0x08, 0x10, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x4C, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x01, 0x3E, 0x54, 0x14, 0x14, 0x00, 0x00, 0x00,

 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 224
 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

