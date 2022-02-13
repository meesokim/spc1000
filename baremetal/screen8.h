//
// screen.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2017  R. Stange <rsta2@o2online.de>
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
#ifndef _circle_screen8_h
#define _circle_screen8_h

#include <circle/device.h>
#include <circle/bcmframebuffer.h>
#include <circle/chargenerator.h>
#include <circle/spinlock.h>
#include <circle/macros.h>
#include <circle/types.h>
#include <circle/screen.h>

#ifdef DEPTH
#undef DEPTH
#define DEPTH 8
typedef u8 TScreenColor8;

#define NORMAL_COLOR16 COLOR16(31, 31, 31)
#define HIGH_COLOR16 COLOR16(31, 0, 0)
#define HALF_COLOR16 COLOR16(0, 0, 31)

struct TScreenStatus8
{
	TScreenColor8 *pContent;
	unsigned nSize;
	unsigned nState;
	unsigned nScrollStart;
	unsigned nScrollEnd;
	unsigned nCursorX;
	unsigned nCursorY;
	boolean bCursorOn;
	TScreenColor8 Color;
	boolean bInsertOn;
	unsigned nParam1;
	unsigned nParam2;
	boolean bUpdated;
};

#else

#define DEPTH 8
typedef u8 TScreenColor8;

// really ((green) & 0x3F) << 5, but to have a 0-31 range for all colors
#define COLOR16(red, green, blue) (((red)&0x1F) << 11 | ((green)&0x3F) << 6 | ((blue)&0x1F))

// BGRA (was RGBA with older firmware)
#define COLOR32(red, green, blue, alpha) (((red)&0xFF) | ((green)&0xFF) << 8 | ((blue)&0xFF) << 16 | ((alpha)&0xFF) << 24)

#define BLACK_COLOR 0

struct TScreenStatus
{
	TScreenColor8 *pContent;
	unsigned nSize;
	unsigned nState;
	unsigned nScrollStart;
	unsigned nScrollEnd;
	unsigned nCursorX;
	unsigned nCursorY;
	boolean bCursorOn;
	TScreenColor8 Color;
	boolean bInsertOn;
	unsigned nParam1;
	unsigned nParam2;
	boolean bUpdated;
};

#endif



class CScreenDevice8 : public CDevice
{
public:
	CScreenDevice8 (unsigned nWidth, unsigned nHeight, boolean bVirtual = FALSE);
	~CScreenDevice8 (void);

	boolean Initialize (void);

	// size in pixels
	unsigned GetWidth (void) const;
	unsigned GetHeight (void) const;
	TScreenColor8 *GetBuffer(void) const;

	// size in characters
	unsigned GetColumns (void) const;
	unsigned GetRows (void) const;

	TScreenStatus8 GetStatus (void);
	int GetDepth(void) const;
	void SetPalette(u8 num, u16 color);
	void SetPalette(u8 num, u32 color);
	void UpdatePalette(void);
	
	boolean SetStatus (TScreenStatus8 Status);	// returns FALSE on failure

	int Write (const void *pBuffer, unsigned nCount);

	void SetPixel (unsigned nPosX, unsigned nPosY, TScreenColor8 Color);
	void SetXY (int x, int y);
	TScreenColor8 GetPixel (unsigned nPosX, unsigned nPosY);

	void Rotor (unsigned nIndex,		// 0..3
		    unsigned nCount);		// 0..3

	void Write (char chChar);
private:

	void CarriageReturn (void);
	void ClearDisplayEnd (void) MAXOPT;
	void ClearLineEnd (void);
	void CursorDown (void);
	void CursorHome (void);
	void CursorLeft (void);
	void CursorMove (unsigned nRow, unsigned nColumn);
	void CursorRight (void);
	void CursorUp (void);
	void DeleteChars (unsigned nCount);
	void DeleteLines (unsigned nCount);
	void DisplayChar (char chChar);
	void EraseChars (unsigned nCount);
	void InsertLines (unsigned nCount);
	void InsertMode (boolean bBegin);
	void NewLine (void);
	void ReverseScroll (void);
	void SetCursorMode (boolean bVisible);
	void SetScrollRegion (unsigned nStartRow, unsigned nEndRow);
	void SetStandoutMode (unsigned nMode);
	void Tabulator (void);

	void Scroll (void) MAXOPT;

	void DisplayChar (char chChar, unsigned nPosX, unsigned nPosY, TScreenColor8 Color);
	void EraseChar (unsigned nPosX, unsigned nPosY);
	void InvertCursor (void);

private:
	unsigned	 m_nInitWidth;
	unsigned	 m_nInitHeight;
	boolean		 m_bVirtual;
	CBcmFrameBuffer	*m_pFrameBuffer;
	CCharGenerator	 m_CharGen;
	TScreenColor8  	*m_pBuffer;
	unsigned	 m_nSize;
	unsigned	 m_nPitch;
	unsigned	 m_nWidth;
	unsigned	 m_nHeight;
	unsigned	 m_nUsedHeight;
	unsigned	 m_nState;
	unsigned	 m_nScrollStart;
	unsigned	 m_nScrollEnd;
	unsigned	 m_nCursorX;
	unsigned	 m_nCursorY;
	boolean		 m_bCursorOn;
	TScreenColor8	 m_Color;
	boolean		 m_bInsertOn;
	unsigned	 m_nParam1;
	unsigned	 m_nParam2;
	boolean		 m_bUpdated;
	CSpinLock	 m_SpinLock;
};

#endif
