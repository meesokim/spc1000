#ifndef _circle_startup_h
#define _circle_startup_h

#include <stdlib.h>

#define EXIT_HALT   0
#define EXIT_REBOOT 1

inline void halt() { exit(0); }
inline void reboot() { exit(0); }

#endif
