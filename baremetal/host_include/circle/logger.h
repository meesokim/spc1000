#ifndef _circle_logger_h
#define _circle_logger_h

#include <circle/types.h>
#include <circle/timer.h>

enum TFilterLevel {
    LogNone,
    LogDebug,
    LogNotice,
    LogWarning,
    LogError
};

enum TLogSource {
    FromKernel
};

class CLogger {
public:
    CLogger(TFilterLevel level, CTimer *pTimer) {}
    boolean Initialize(void *pTarget = 0) { return TRUE; }
    void Write(TLogSource source, TFilterLevel level, const char *format, ...) {}
};

#endif
