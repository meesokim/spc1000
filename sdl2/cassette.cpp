typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

#include "cassette.h"
#include <stdio.h>
#include <stdlib.h>

// #define PULSE ((1349-200)*0.5)
#define PULSE 7
char Cassette::read(uint32_t cycles, uint8_t wait) {
    char val = 0;
    int diff = cycles - old_cycles;
    if (diff > 4 * PULSE * 90)
    {
        mark = -3;
        inv_time = 0;
    } else if (mark < -2)
    {
        mark++;
    } else if (len && mark < 0)
    {
        // mark = (type == TYPE_CHARBIN ? (tape[pos] == '1' ? 1 : 0) : (tape[pos>>3] & (1 << (pos % 8) ? 1 : 0)));
        mark = (tape[pos] == '1' ? 1:0);
        old_time = cycles;
        inv_time = cycles + 75;
        end_time = inv_time + 90 + PULSE * wait * (mark+1);
        if (pos < 100)
            printf("%d--[%d]%d/%d,%d\n", mark, pos, inv_time - cycles, end_time - cycles, wait);
            // printf("%d\n", mark);
        if (++pos >= len)
            pos = 0;
    }
    if (mark > -1)
    {
        if (cycles < inv_time)
            val = 0;
        else if (cycles < end_time)
            val = 1;
    }
    if (pos < 100 &&inv_time > 0 && val != mark)
        printf("%d(%d)%c\n", val, cycles - old_time, val != mark ? '*' : 0);
    if (cycles > end_time)
        mark = -1;
    old_cycles = cycles;
    return val;
}

void Cassette::write(char ch)
{
    // printf("%d", ch);
}

void Cassette::load(const char *name) 
{
    if (!name)
    {
        name = files[file_index].c_str();
        printf("%s\n", name);
    } 
    FILE *f = fopen(name, "rb");
    if (f != NULL) {
        len = pos = 0;
        int char_count = 0;
        for(int i = 0; i < 2; i++)
            tape[pos++] = '0';
        while(!feof(f)) 
        {
            if(pos==2) 
            {
                while(fgetc(f)=='0') len=-1;
                if (len < 0)
                {
                    tape[pos++]='1';
                    len = pos;
                }
            }
            char ch = fgetc(f);
            if (ch != '1' && ch != '0' && ch == ' ')
                char_count++;
            len++;
            tape[pos] = ch == '1' ? '1' : '0';
            pos = (pos++ == TAPE_SIZE ? 0 : pos);
        }
        // char data[100];
        // strncpy(data, tape, 99);
        // printf("%s\n", data);
        if (len > pos)
            len = pos;
        if (char_count * 1.2 > pos)
            type = TYPE_CHARBIN;
        else
            type = TYPE_BINARY;
        fclose(f);
        pos = 0;
        printf("filename:%s(%d)\n", name, len);
    }   
}

void Cassette::loaddir(const char *dirname)
{
    for (const auto & entry : fs::directory_iterator(dirname))
        if (!entry.path().extension().compare(".tap"))
            files.push_back(entry.path());
    load(files[0].c_str());
}