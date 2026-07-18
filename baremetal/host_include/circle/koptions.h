#ifndef _circle_koptions_h
#define _circle_koptions_h

#include <circle/logger.h>

class CKernelOptions {
public:
    CKernelOptions() {}
    TFilterLevel GetLogLevel() { return LogNone; }
};

#endif
