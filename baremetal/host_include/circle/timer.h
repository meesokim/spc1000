#ifndef _circle_timer_h
#define _circle_timer_h

#include <circle/types.h>
#include <circle/interrupt.h>

class CTimer {
public:
    CTimer(CInterruptSystem *pInterrupt = nullptr) {}
    boolean Initialize() { return TRUE; }
    void MsDelay(unsigned nMs) {}
    void usDelay(unsigned nUs) {}
};

#endif
