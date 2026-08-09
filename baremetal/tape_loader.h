#ifndef TAPE_LOADER_H_
#define TAPE_LOADER_H_

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _TapeLoaderConfig
{
    // [sync]
    int zero_skip;
    char sync_pattern[9];
    int precursor_zeros;
    int start_offset;

    // [tape]
    bool rewind_on_reset;
    bool auto_load;
} TapeLoaderConfig;

void TapeLoaderConfig_InitDefaults(TapeLoaderConfig *cfg);
bool TapeLoaderConfig_Parse(TapeLoaderConfig *cfg, const char *text);

#ifdef __cplusplus
}
#endif

#endif // TAPE_LOADER_H_
