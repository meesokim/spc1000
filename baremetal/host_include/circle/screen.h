#ifndef _circle_screen_h
#define _circle_screen_h

#include <circle/types.h>
#include <circle/bcmframebuffer.h>

class CScreenDevice {
public:
    CScreenDevice(unsigned nWidth, unsigned nHeight);
    ~CScreenDevice();

    boolean Initialize();
    void Write(const char *pBuffer, unsigned nLength);
    CBcmFrameBuffer *GetFrameBuffer();

private:
    unsigned m_nWidth;
    unsigned m_nHeight;
    CBcmFrameBuffer *m_pFB;
};

#endif
