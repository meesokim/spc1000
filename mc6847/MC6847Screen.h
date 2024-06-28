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
#ifndef _SPC1000_MC6847Screen_h
#define _SPC1000_MC6847Screen_h

#include <circle/bcmframebuffer.h>
#include <circle/screen.h>
#include <circle/types.h>

class CMC6847Screen
{
public:
	CMC6847Screen (void);
	~CMC6847Screen (void);

	boolean Initialize (u8 *pVideoMem);

	void Update (boolean flash);

private:
	CBcmFrameBuffer	*m_pFrameBuffer;
	u32		*m_pBuffer;		// Address of frame buffer
	unsigned	 m_scrTable[256][256];	// lookup table
	u8		*m_pVideoMem;		// Spectrum video memory
};

#endif
