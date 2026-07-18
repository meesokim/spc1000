#ifndef _circle_bcmframebuffer_h
#define _circle_bcmframebuffer_h

#include <circle/types.h>

class CBcmFrameBuffer {
public:
    CBcmFrameBuffer(unsigned nWidth, unsigned nHeight);
    ~CBcmFrameBuffer();

    void *GetBuffer() { return m_pBuffer; }
    unsigned GetWidth() { return m_nWidth; }
    unsigned GetHeight() { return m_nHeight; }
    unsigned GetPitch() { return m_nWidth * 2; } // 16bpp (2 bytes per pixel)

private:
    unsigned m_nWidth;
    unsigned m_nHeight;
    u16 *m_pBuffer;
};

#endif
