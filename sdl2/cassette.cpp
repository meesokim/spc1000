typedef unsigned int uint32_t;

#include "cassette.h"
#include <stdio.h>
#include <stdlib.h>

#define PULSE 4000 >> 5
char Cassette::read(uint32_t ms) {
    char val = 0;
    do {
        char mark = (type == TYPE_CHARBIN ? (tape[pos] == '1' ? 1 : 0) : (tape[pos>>3] & (1 << (pos % 8) ? 1 : 0)));
        if (mark) 
        {
            if (ms - prev > 4 * PULSE) 
            {
                prev += 4 * PULSE;
                pos ++;
            }
            else if (ms - prev > 2 * PULSE)
            {
                prev += 2 * PULSE;
                val = 0;
                break;
            }
            else 
            {
                prev = ms;
                val = 1;                
                break;
            }
        }
        else 
        {
            if (ms - prev > 2 * PULSE)
            {
                prev += 2 * PULSE;
                pos ++;
            }
            else if (ms - prev > PULSE)
            {
                prev += PULSE;
                val = 0; 
                break;
            }
            else 
            {
                prev = ms;
                val = 1;
                break;
            }
        }
    } while(1);
    return val;
}

void Cassette::write(char ch)
{
    printf("%d", ch);
}

void Cassette::load(const char *name) 
{
    FILE *f = fopen(name, "rb");
    if (f != NULL) {
        len = pos = 0;
        int char_count = 0;
        while(!feof(f)) 
        {
            char ch = fgetc(f);
            if (ch != '1' && ch != '0' && ch == ' ')
                char_count++;
            len++;
            tape[pos++] = ch;
            pos = (pos == TAPE_SIZE ? 0 : pos);
        }
        if (len > pos)
            len = pos;
        if (char_count * 1.2 > pos)
            type = TYPE_CHARBIN;
        else
            type = TYPE_BINARY;
        fclose(f);
        pos = 0;
    }   
}