#ifndef _circle_devicenameservice_h
#define _circle_devicenameservice_h

#include <circle/types.h>
#include <circle/usb/usbkeyboard.h>
#include <string.h>

extern CUSBKeyboardDevice *g_pKeyboardDevice;

class CDeviceNameService {
public:
    CDeviceNameService() {}
    void *GetDevice(const char *pName, boolean bWait) {
        if (strcmp(pName, "ukbd1") == 0) {
            if (!g_pKeyboardDevice) {
                g_pKeyboardDevice = new CUSBKeyboardDevice();
            }
            return g_pKeyboardDevice;
        }
        return nullptr;
    }
};

#endif
