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

	m_pFrameBuffer = new CBcmFrameBuffer (344, 272, 4);
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
	// m_Screen.SetPalette(c++, (u32)COLOR32(0x00, 0x00, 0x00, 0xff)); /* BLACK */
	// m_Screen.SetPalette(c++, (u32)COLOR32(0x07, 0xff, 0x00, 0xff)); /* GREEN */ 
	// m_Screen.SetPalette(c++, (u32)COLOR32(0xff, 0xff, 0x00, 0xff)); /* YELLOW */
	// m_Screen.SetPalette(c++, (u32)COLOR32(0x3b, 0x08, 0xff, 0xff)); /* BLUE */
	// m_Screen.SetPalette(c++, (u32)COLOR32(0xcc, 0x00, 0x3b, 0xff)); /* RED */
	// m_Screen.SetPalette(c++, (u32)COLOR32(0xff, 0xff, 0xff, 0xff)); /* BUFF */
	// m_Screen.SetPalette(c++, (u32)COLOR32(0x07, 0xe3, 0x99, 0xff)); /* CYAN */
	// m_Screen.SetPalette(c++, (u32)COLOR32(0xff, 0x1c, 0xff, 0xff)); /* MAGENTA */
	// m_Screen.SetPalette(c++, (u32)COLOR32(0xff, 0x81, 0x00, 0xff)); /* ORANGE */
	
	// m_Screen.SetPalette(c++, (u32)COLOR32(0x07, 0xff, 0x00, 0xff)); /* GREEN */
	// m_Screen.SetPalette(c++, (u32)COLOR32(0xff, 0xff, 0xff, 0xff)); /* BUFF */
	
	// m_Screen.SetPalette(c++, (u32)COLOR32(0x00, 0x3f, 0x00, 0xff)); /* ALPHANUMERIC DARK GREEN */
	// m_Screen.SetPalette(c++, (u32)COLOR32(0x07, 0xff, 0x00, 0xff)); /* ALPHANUMERIC BRIGHT GREEN */ 
	// m_Screen.SetPalette(c++, (u32)COLOR32(0x91, 0x00, 0x00, 0xff)); /* ALPHANUMERIC DARK ORANGE */
	// m_Screen.SetPalette(c++, (u32)COLOR32(0xff, 0x81, 0x00, 0xff)); /* ALPHANUMERIC BRIGHT ORANGE */		
	// m_Screen.SetPalette(0xff,(u32)COLOR32(0xff, 0xff, 0xff, 0xff));
	// m_Screen.SetPalette(0x46,(u32)COLOR32(0xff, 0x00, 0x00, 0xff));
	if (!m_pFrameBuffer->Initialize()) {
		return FALSE;
	}

	m_pBuffer = (u32 *) (uintptr) m_pFrameBuffer->GetBuffer();
	assert (m_pBuffer != 0);

	memset(m_pBuffer, 0xFF, m_pFrameBuffer->GetSize());

	// Create a lookup table for draw the screen Faster Than Light :)
	for (int attr = 0; attr < 128; attr++) {
		unsigned ink = attr & 0x07;
		unsigned paper = (attr & 0x78) >> 3;
		if (attr & 0x40) {
			ink |= 0x08;
		}
		for (int idx = 0; idx < 256; idx++) {
			for (unsigned mask = 0x80; mask > 0; mask >>= 1) {
				m_scrTable[attr][idx] <<= 4;
				m_scrTable[attr + 128][idx] <<= 4;
				if (idx & mask) {
					m_scrTable[attr][idx] |= ink;
					m_scrTable[attr + 128][idx] |= paper;
				} else {
					m_scrTable[attr][idx] |= paper;
					m_scrTable[attr + 128][idx] |= ink;
				}
			}
			m_scrTable[attr][idx] = bswap32(m_scrTable[attr][idx]);
			m_scrTable[attr + 128][idx] = bswap32(m_scrTable[attr + 128][idx]);
		}
	}

	return TRUE;
}

void CMC6847Screen::Update (boolean flash)
{
	assert (m_pBuffer != 0);
	assert (m_pVideoMem != 0);

	int bufIdx = 1413;
	int attr = 6144;
	for (int addr = 0; addr < 256; addr += 32) {
		for (int col = 0; col < 32; col++) {
			unsigned char color = m_pVideoMem[attr++];
			if (!flash)
				color &= 0x7F;
			m_pBuffer[bufIdx + col] = m_scrTable[color][m_pVideoMem[addr + col]];
			m_pBuffer[bufIdx + col + 44] = m_scrTable[color][m_pVideoMem[addr + col + 256]];
			m_pBuffer[bufIdx + col + 88] = m_scrTable[color][m_pVideoMem[addr + col + 512]];
			m_pBuffer[bufIdx + col + 132] = m_scrTable[color][m_pVideoMem[addr + col + 768]];
			m_pBuffer[bufIdx + col + 176] = m_scrTable[color][m_pVideoMem[addr + col + 1024]];
			m_pBuffer[bufIdx + col + 220] = m_scrTable[color][m_pVideoMem[addr + col + 1280]];
			m_pBuffer[bufIdx + col + 264] = m_scrTable[color][m_pVideoMem[addr + col + 1536]];
			m_pBuffer[bufIdx + col + 308] = m_scrTable[color][m_pVideoMem[addr + col + 1792]];
		}
		bufIdx += 352;
	}
	for (int addr = 2048; addr < 2304; addr += 32) {
		for (int col = 0; col < 32; col++) {
			unsigned char color = m_pVideoMem[attr++];
			if (!flash)
				color &= 0x7F;
			m_pBuffer[bufIdx + col] = m_scrTable[color][m_pVideoMem[addr + col]];
			m_pBuffer[bufIdx + col + 44] = m_scrTable[color][m_pVideoMem[addr + col + 256]];
			m_pBuffer[bufIdx + col + 88] = m_scrTable[color][m_pVideoMem[addr + col + 512]];
			m_pBuffer[bufIdx + col + 132] = m_scrTable[color][m_pVideoMem[addr + col + 768]];
			m_pBuffer[bufIdx + col + 176] = m_scrTable[color][m_pVideoMem[addr + col + 1024]];
			m_pBuffer[bufIdx + col + 220] = m_scrTable[color][m_pVideoMem[addr + col + 1280]];
			m_pBuffer[bufIdx + col + 264] = m_scrTable[color][m_pVideoMem[addr + col + 1536]];
			m_pBuffer[bufIdx + col + 308] = m_scrTable[color][m_pVideoMem[addr + col + 1792]];
		}
		bufIdx += 352;
	}
	for (int addr = 4096; addr < 4352; addr += 32) {
		for (int col = 0; col < 32; col++) {
			unsigned char color = m_pVideoMem[attr++];
			if (!flash)
				color &= 0x7F;
			m_pBuffer[bufIdx + col] = m_scrTable[color][m_pVideoMem[addr + col]];
			m_pBuffer[bufIdx + col + 44] = m_scrTable[color][m_pVideoMem[addr + col + 256]];
			m_pBuffer[bufIdx + col + 88] = m_scrTable[color][m_pVideoMem[addr + col + 512]];
			m_pBuffer[bufIdx + col + 132] = m_scrTable[color][m_pVideoMem[addr + col + 768]];
			m_pBuffer[bufIdx + col + 176] = m_scrTable[color][m_pVideoMem[addr + col + 1024]];
			m_pBuffer[bufIdx + col + 220] = m_scrTable[color][m_pVideoMem[addr + col + 1280]];
			m_pBuffer[bufIdx + col + 264] = m_scrTable[color][m_pVideoMem[addr + col + 1536]];
			m_pBuffer[bufIdx + col + 308] = m_scrTable[color][m_pVideoMem[addr + col + 1792]];
		}
		bufIdx += 352;
	}
}
