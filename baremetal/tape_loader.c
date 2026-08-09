#include "tape_loader.h"
#include <stdio.h>

void TapeLoaderConfig_InitDefaults(TapeLoaderConfig *cfg)
{
    memset(cfg, 0, sizeof(TapeLoaderConfig));

    cfg->zero_skip = 50;
    strcpy(cfg->sync_pattern, "11000000");
    cfg->precursor_zeros = 15;
    cfg->start_offset = 2;

    cfg->rewind_on_reset = true;
    cfg->auto_load = true;
}

static const char *tl_skip_ws(const char *p)
{
    while (*p == ' ' || *p == '\t' || *p == '\r')
        p++;
    return p;
}

static bool tl_starts_with(const char *s, const char *prefix)
{
    while (*prefix)
    {
        if (*s != *prefix) return false;
        s++;
        prefix++;
    }
    return true;
}

static int tl_parse_int(const char *s, int base)
{
    while (*s == ' ' || *s == '\t') s++;
    if (base == 16 && (tl_starts_with(s, "0x") || tl_starts_with(s, "0X")))
        s += 2;

    int sign = 1;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') { s++; }

    int value = 0;
    while (*s)
    {
        int digit = -1;
        char c = *s;
        if (c >= '0' && c <= '9')
            digit = c - '0';
        else if (base == 16)
        {
            if (c >= 'a' && c <= 'f')
                digit = 10 + (c - 'a');
            else if (c >= 'A' && c <= 'F')
                digit = 10 + (c - 'A');
        }
        if (digit < 0 || digit >= base)
            break;
        value = value * base + digit;
        s++;
    }
    return sign * value;
}

bool TapeLoaderConfig_Parse(TapeLoaderConfig *cfg, const char *text)
{
    if (!cfg || !text) return false;

    // Keep defaults; override from file.
    const char *p = text;
    char section[32] = "";

    while (*p)
    {
        p = tl_skip_ws(p);
        if (*p == '\0') break;

        if (*p == '\n')
        {
            p++;
            continue;
        }

        if (*p == ';' || *p == '#')
        {
            while (*p && *p != '\n') p++;
            continue;
        }

        if (*p == '[')
        {
            p++;
            int i = 0;
            while (*p && *p != ']' && i < (int)sizeof(section) - 1)
                section[i++] = *p++;
            section[i] = '\0';
            while (*p && *p != '\n') p++;
            continue;
        }

        // Parse key=value
        const char *key = p;
        while (*p && *p != '=' && *p != '\n' && *p != ';') p++;
        if (*p != '=')
        {
            while (*p && *p != '\n') p++;
            continue;
        }
        int key_len = (int)(p - key);
        p++; // skip '='
        p = tl_skip_ws(p);
        const char *val = p;
        while (*p && *p != '\n' && *p != ';') p++;
        int val_len = (int)(p - val);

        char keybuf[64], valbuf[64];
        if (key_len >= (int)sizeof(keybuf)) key_len = sizeof(keybuf) - 1;
        if (val_len >= (int)sizeof(valbuf)) val_len = sizeof(valbuf) - 1;
        memcpy(keybuf, key, key_len); keybuf[key_len] = '\0';
        memcpy(valbuf, val, val_len); valbuf[val_len] = '\0';

        // Trim trailing spaces in value
        int vlen = (int)strlen(valbuf);
        while (vlen > 0 && (valbuf[vlen - 1] == ' ' || valbuf[vlen - 1] == '\t' || valbuf[vlen - 1] == '\r'))
            valbuf[--vlen] = '\0';

        if (strcmp(section, "sync") == 0)
        {
            if (strcmp(keybuf, "zero_skip") == 0)
                cfg->zero_skip = tl_parse_int(valbuf, 10);
            else if (strcmp(keybuf, "pattern") == 0)
            {
                if (vlen == 8)
                    strcpy(cfg->sync_pattern, valbuf);
            }
            else if (strcmp(keybuf, "precursor_zeros") == 0)
                cfg->precursor_zeros = tl_parse_int(valbuf, 10);
            else if (strcmp(keybuf, "start_offset") == 0)
                cfg->start_offset = tl_parse_int(valbuf, 10);
        }
        else if (strcmp(section, "tape") == 0)
        {
            if (strcmp(keybuf, "rewind_on_reset") == 0)
                cfg->rewind_on_reset = (tl_parse_int(valbuf, 10) != 0);
            else if (strcmp(keybuf, "auto_load") == 0)
                cfg->auto_load = (tl_parse_int(valbuf, 10) != 0);
        }

        while (*p && *p != '\n') p++;
    }

    return true;
}
