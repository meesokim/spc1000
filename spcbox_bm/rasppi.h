#ifndef ARM_IO_BASE
#if RASPPI == 1
#define ARM_IO_BASE 0x20000000
#elif RASPPI <= 3
#define ARM_IO_BASE 0x3F000000
#else
#define ARM_IO_BASE 0xFE000000
#endif
#endif