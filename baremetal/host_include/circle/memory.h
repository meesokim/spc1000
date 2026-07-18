#ifndef _circle_memory_h
#define _circle_memory_h

#include <circle/types.h>

class CMemorySystem {
public:
    CMemorySystem(boolean bCoherent = TRUE) {}
    static CMemorySystem* Get() { return nullptr; }
};

#endif
