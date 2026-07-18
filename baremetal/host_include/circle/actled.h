#ifndef _circle_actled_h
#define _circle_actled_h

#include <circle/types.h>

class CActLED {
public:
    CActLED() {}
    static CActLED* Get() { return nullptr; }
    void On() {}
    void Off() {}
    void Blink(unsigned nCount = 0) {}
};

#endif
