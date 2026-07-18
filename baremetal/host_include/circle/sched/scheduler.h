#ifndef _circle_sched_scheduler_h
#define _circle_sched_scheduler_h

#include <circle/types.h>

class CScheduler {
public:
    CScheduler();
    void Yield();
    void MsSleep(unsigned nMs);
};

#endif
