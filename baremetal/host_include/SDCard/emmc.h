#ifndef _circle_sdcard_emmc_h
#define _circle_sdcard_emmc_h

#include <circle/types.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/actled.h>

class CEMMCDevice {
public:
    CEMMCDevice(CInterruptSystem *pInterrupt = nullptr, CTimer *pTimer = nullptr, CActLED *pLED = nullptr) {}
    boolean Initialize() { return TRUE; }
};

#endif
