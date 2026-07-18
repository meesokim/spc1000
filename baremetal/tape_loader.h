#ifndef TAPE_LOADER_H_
#define TAPE_LOADER_H_

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_INJECTIONS 8
#define MAX_EXTRA_PATCHES 8

typedef struct _TapeLoaderConfig
{
    // [sync]
    int zero_skip;
    char sync_pattern[9];
    int precursor_zeros;
    int start_offset;

    // [injection]
    int injection_count;
    int injection_pos[MAX_INJECTIONS];
    const char *injection_bits[MAX_INJECTIONS];
    bool injection_done[MAX_INJECTIONS];

    // [checksum_bypass]
    bool checksum_bypass_enabled;
    int checksum_patch_count;
    unsigned short checksum_patch_addr[MAX_EXTRA_PATCHES];
    unsigned char checksum_patch_value[MAX_EXTRA_PATCHES];

    // [tape]
    bool rewind_on_reset;
    bool auto_load;
} TapeLoaderConfig;

void TapeLoaderConfig_InitDefaults(TapeLoaderConfig *cfg);
bool TapeLoaderConfig_Parse(TapeLoaderConfig *cfg, const char *text);
void TapeLoaderConfig_ResetInjections(TapeLoaderConfig *cfg);

#ifdef __cplusplus
}
#endif

#endif // TAPE_LOADER_H_
