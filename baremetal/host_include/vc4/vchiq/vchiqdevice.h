#ifndef _vc4_vchiq_vchiqdevice_h
#define _vc4_vchiq_vchiqdevice_h

#include <circle/types.h>
#include <circle/memory.h>
#include <circle/interrupt.h>

class CVCHIQDevice {
public:
    CVCHIQDevice(CMemorySystem *pMemorySystem = nullptr, CInterruptSystem *pInterrupt = nullptr) {}
    boolean Initialize() { return TRUE; }
};

#endif
