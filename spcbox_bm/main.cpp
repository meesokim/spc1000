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

#define printf(fmt, ...) (0)
extern "C"
{
}
#include "spcbox.h"
SpcBox *sbox;
extern "C"
{
    #include "tap.h"
    __attribute__((interrupt("FIQ"))) interrupt_fiq() {
        volatile register uint32_t g = GPIO_GET();
        if (!((g = GPIO_GET()) & RPSPC_EXT))
        {
            volatile register uint8_t addr = (g >> RPSPC_A0_PIN) & 3;
            if (!(g & RPSPC_WR)) {
                sbox->write(addr, GPIO_GET());
            } else {
                GPIO_CLR(0xff);
                GPIO_SET(sbox->read(addr));
            }
            while(!(GPIO_GET() & RPSPC_EXT));
            GPIO_SET(0x0);
        }
        PUT32(ARM_GPIO_GPEDS0, RPSPC_EXT);
    }
}
volatile uint32_t a = 0;
int main(void) {
    // // mount("SD:");
	volatile SpcBox *sbox;
	GPIO_SEL0(IOSEL0);
    GPIO_SEL1(0);
    GPIO_SEL2(0);
    GPIO_CLR(0xff);
    // PUT32(BCM2835_GPIO_PADS, BCM2835_PAD_PASSWRD | BCM2835_PAD_HYSTERESIS_ENABLED | BCM2835_PAD_DRIVE_16mA);
    PUT32(ARM_IC_FIQ_CONTROL, 0x80 | ARM_FIQ_GPIO0);
	asm("cpsid i");
	asm("cpsie f");
    PUT32(ARM_GPIO_GPEDS0, RPSPC_EXT);
    PUT32(ARM_GPIO_GPFEN0, RPSPC_EXT);
    TapeFiles *tape = new TapeFiles();
    tape->initialize((const char*)tap_zip, sizeof(tap_zip));
    sbox = new SpcBox(tape); 
    while(true) {
        sbox->execute();
    }
}
