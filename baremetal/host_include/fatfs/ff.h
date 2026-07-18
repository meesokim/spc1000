#ifndef _fatfs_ff_h
#define _fatfs_ff_h

#include <stdio.h>
#include <circle/types.h>

typedef int FRESULT;
#define FR_OK 0

#define FA_READ          0x01
#define FA_WRITE         0x02
#define FA_OPEN_EXISTING 0x00
#define FA_OPEN_ALWAYS   0x10

typedef unsigned int UINT;
typedef unsigned int DWORD;

struct FATFS {
    int dummy;
};

struct FIL {
    FILE *fh;
};

FRESULT f_open(FIL *fp, const char *path, byte mode);
FRESULT f_close(FIL *fp);
FRESULT f_read(FIL *fp, void *buff, unsigned btr, unsigned *br);
FRESULT f_write(FIL *fp, const void *buff, unsigned btw, unsigned *bw);
FRESULT f_lseek(FIL *fp, unsigned long ofs);
unsigned long f_size(FIL *fp);
FRESULT f_mount(FATFS *fs, const char *path, byte opt);

#endif
