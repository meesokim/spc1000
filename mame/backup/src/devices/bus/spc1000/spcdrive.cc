#include <stdio.h>
#include "spcbus.h"
#include "spcbox.h"
#include "tap.h"

SpcBox *sbox;

extern "C" {
    void spcinit(char path[]) 
    {
        printf("init(%s)\n", path);
	    if (!::sbox) {
			TapeFiles *tape = new TapeFiles();
			printf("filename:%s\n", filename());
			if (path)
				tape->initialize(path);
			else if (size>0) 
				tape->initialize(files, size);
			else
				tape->initialize((const char*)tap_zip, sizeof(tap_zip));
	        ::sbox = new SpcBox(tape);
		}
		sbox->initialize();
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
