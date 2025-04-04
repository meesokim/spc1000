extern "C" {
#include "spcbus.h"
}

extern "C" {
void init(char path[]) 
{
    printf("init(%s)\n", path);
}

unsigned char spcread(int cmd, unsigned short addr) 
{
    unsigned char b = 0;
    bool ret = false;
    // printf("R%04x:%02x\n", addr, b);
    return b;
}

void spcwrite(int cmd, unsigned short addr, unsigned char value)
{
    // printf("W%04x:%02x\n", addr, value);
    bool ret = false;
}

void reset() 
{

}
}
