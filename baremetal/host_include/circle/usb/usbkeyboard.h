#ifndef _circle_usb_usbkeyboard_h
#define _circle_usb_usbkeyboard_h

#include <circle/types.h>

typedef void TKeyStatusHandlerRaw(unsigned char ucModifiers, const unsigned char RawKeys[6]);

class CUSBKeyboardDevice {
public:
    CUSBKeyboardDevice() : m_pHandler(nullptr) {}
    void RegisterKeyStatusHandlerRaw(TKeyStatusHandlerRaw *pHandler) {
        m_pHandler = pHandler;
    }
    TKeyStatusHandlerRaw *m_pHandler;
};

#endif
