#include <stdint.h>
#include <cstring>
#include <cstdio>
#include "pireg.h"

#define RPSPC_D0	(1 << 0)
#define MD00_PIN	0
#define RPSPC_WR	(1 << 14)
#define RPSPC_EXT	(1 << 17)
#define RPSPC_RST   (1 << 27)
#define RPSPC_CLK	(1 << 18)
#define RPSPC_A0	(1 << 22)
#define RPSPC_A1	(1 << 23)
#define RPSPC_A11	(1 << 24)
#define RPSPC_A6	(1 << 25)
#define RPSPC_A0_PIN 22

#define ADDR  (RPSPC_A0 | RPSPC_A1)
#define ADDR0 0
#define ADDR1 RPSPC_A0
#define ADDR2 RPSPC_A1

#define READY 			0
#define READ_FOR_DATA  	1
#define DATA_VALID 		2
#define RECEIVED		3

#include "fatfs/ff.h"

#define DRIVE		"SD:"

// #include <thread>
using namespace std;

#include <map>
#include <vector>
#include <iostream>
// #define THREAD                    

// #define printf(fmt, ...) (0)
#include "tap.h"
#include "spcbox.h"

extern "C"
{
    extern int mount(const char *source);    
	__attribute__((interrupt("FIQ"))) interrupt_fiq() {
#if 0
		volatile register uint32_t a = GPIO_GET();
        // if (!(GPIO_GET() & RPSPC_EXT))
        {
            volatile register uint8_t addr = (a >> RPSPC_A0_PIN) & 3;
            if (a & RPSPC_WR) {
                GPIO_CLR(0xff);
                GPIO_SET(sbox->read(addr));
            } else {
                sbox->write(addr, a);
                sbox->execute();
            }
            while(!(GPIO_GET() & RPSPC_EXT));
        }
        PUT32(ARM_GPIO_GPEDS0, RPSPC_EXT);
#endif
    };
}
volatile uint32_t a = 0;
int main(void) {
    // // mount("SD:");
	volatile SpcBox *sbox;
    TapeFiles *tape = new TapeFiles();
    tape->initialize((const char*)tap_zip, sizeof(tap_zip));
    sbox = new SpcBox(tape); 
	GPIO_SEL0(IOSEL0);
    GPIO_SEL1(0);
    GPIO_SEL2(0);
    GPIO_CLR(0xff);
    PUT32(BCM2835_GPIO_PADS, BCM2835_PAD_PASSWRD | BCM2835_PAD_HYSTERESIS_ENABLED | BCM2835_PAD_DRIVE_16mA);
    // PUT32(ARM_IC_FIQ_CONTROL, 0x80 | ARM_FIQ_GPIO0);
    // PUT32(ARM_GPIO_GPEDS0, RPSPC_EXT);
    // PUT32(ARM_GPIO_GPFEN0, RPSPC_EXT);
	asm("cpsid i");
	// asm("cpsid f");
    uint8_t data[4] = {0,1,2,3};
    GPIO_SEL0(0);
    while(true) {
#if 1
        if (!((a = GPIO_GET()) & RPSPC_EXT))
        {
            int addr = (a >> RPSPC_A0_PIN) & 3;
            GPIO_CLR(0xff);
            if (a & RPSPC_WR) {
	            GPIO_SEL0(IOSEL0);
                GPIO_SET(sbox->read(addr));
            } else {
            	GPIO_SEL0(0);
                a = GPIO_GET();                
                a = GPIO_GET();                
                a = GPIO_GET();                
                a = GPIO_GET();                
                sbox->write(addr, GPIO_GET());
                sbox->execute();
            }
            while(!(GPIO_GET() & RPSPC_EXT));
        } 
        // else if (!(a & RPSPC_RST)) {
        //     while(!(GPIO_GET() & RPSPC_RST));
        //     sbox->initialize();
        // }
        // while((GPIO_GET() & RPSPC_EXT));
        // GPIO_CLR(0xff);
#else
        if (!((a = GPIO_GET()) & RPSPC_EXT))
        {
            int addr = (a >> RPSPC_A0_PIN) & 3;
            if (a & RPSPC_WR) {
                GPIO_CLR(0xff);
	            GPIO_SEL0(IOSEL0);
                GPIO_SET(data[addr]);
            } else {
                GPIO_SEL0(0);
                a = GPIO_GET();                
                a = GPIO_GET();                
                a = GPIO_GET();                
                a = GPIO_GET();                
                data[addr] = a;
            }
            while(!(GPIO_GET() & RPSPC_EXT));
        }
#endif
    }
}