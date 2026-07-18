#ifndef _circle_usb_usbhcidevice_h
#define _circle_usb_usbhcidevice_h

#include <circle/types.h>
#include <circle/interrupt.h>
#include <circle/timer.h>

class CUSBHCIDevice {
public:
    CUSBHCIDevice(CInterruptSystem *pInterrupt = nullptr, CTimer *pTimer = nullptr, boolean bCoherent = TRUE) {}
    boolean Initialize() { return TRUE; }
    void UpdatePlugAndPlay() {}
};

#endif
