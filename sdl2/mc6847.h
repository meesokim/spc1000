//
// MC6847Screen.h
//
// MC6847 screen emulator code provided by Jose Luis Sanchez
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
#ifndef _SPC1000_MC6847_h
#define _SPC1000_MC6847_h

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

class CMC6847
{
public:
	CMC6847 (void);
	~CMC6847 (void);

	bool Initialize (u8 *pVideoMem);

	void Update ();

	u16 *GetBuffer();

private:
	u16	*m_pBuffer;		// Address of frame buffer
	// u16	m_scrTable[256][256];	// lookup table
    u16 palette[256];
	u8	*m_pVideoMem;		// MC6847 video memory
	int currentPage = 0;    // current text page
	int XWidth = 0;			// stride for Y+1
	int height = 192, width = 256;
	static unsigned char CGROM[];	// CGROM, defined at the end of this file
	int bpp;
	int ATTR_INV = 0x1; // white
	int ATTR_CSS = 0x2; // cyan blue
	int ATTR_SEM = 0x4;
	int ATTR_EXT = 0x8;
	enum colorNum {
		COLOR_BLACK, COLOR_GREEN, COLOR_YELLOW, COLOR_BLUE,
		COLOR_RED, COLOR_BUFF, COLOR_CYAN, COLOR_MAGENTA,
		COLOR_ORANGE, COLOR_CYANBLUE, COLOR_LGREEN, COLOR_DGREEN,COLOR_WHITE };
	u8 cMap[18];
	unsigned char semiGrFont0[16*12];	// semigraphic pattern for mode 0
	unsigned char semiGrFont1[64*12];	// semigraphic pattern for mode 1	
};

#endif
