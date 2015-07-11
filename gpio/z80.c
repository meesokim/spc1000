/*
Copyright (c) 2012-2015 Ben Croston

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "gpio.h"

#define BCM2708_PERI_BASE_DEFAULT   0x20000000
#define BCM2709_PERI_BASE_DEFAULT   0x3f000000
#define GPIO_BASE_OFFSET            0x200000
#define FSEL_OFFSET                 0   // 0x0000
#define SET_OFFSET                  7   // 0x001c / 4
#define CLR_OFFSET                  10  // 0x0028 / 4
#define PINLEVEL_OFFSET             13  // 0x0034 / 4
#define EVENT_DETECT_OFFSET         16  // 0x0040 / 4
#define RISING_ED_OFFSET            19  // 0x004c / 4
#define FALLING_ED_OFFSET           22  // 0x0058 / 4
#define HIGH_DETECT_OFFSET          25  // 0x0064 / 4
#define LOW_DETECT_OFFSET           28  // 0x0070 / 4
#define PULLUPDN_OFFSET             37  // 0x0094 / 4
#define PULLUPDNCLK_OFFSET          38  // 0x0098 / 4

#define PAGE_SIZE  (4*1024)
#define BLOCK_SIZE (4*1024)


typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

static volatile uint32_t *gpio_map;

void short_wait(void)
{
    int i;

    for (i=0; i<150; i++) {    // wait 150 cycles
        __asm volatile("nop");
    }
}

int setup(void)
{
    int mem_fd;
    uint8_t *gpio_mem;
    uint32_t peri_base;
    uint32_t gpio_base;
    unsigned char buf[4];
    FILE *fp;
    char buffer[1024];
    char hardware[1024];
    int found = 0;

    // determine peri_base
    if ((fp = fopen("/proc/device-tree/soc/ranges", "rb")) != NULL) {
        // get peri base from device tree
        fseek(fp, 4, SEEK_SET);
        if (fread(buf, 1, sizeof buf, fp) == sizeof buf) {
            peri_base = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3] << 0;
        }
        fclose(fp);
    } else {
        // guess peri base based on /proc/cpuinfo hardware field
        if ((fp = fopen("/proc/cpuinfo", "r")) == NULL)
            return SETUP_CPUINFO_FAIL;

        while(!feof(fp) && !found) {
            fgets(buffer, sizeof(buffer), fp);
            sscanf(buffer, "Hardware	: %s", hardware);
            if (strcmp(hardware, "BCM2708") == 0 || strcmp(hardware, "BCM2835") == 0) {
                // pi 1 hardware
                peri_base = BCM2708_PERI_BASE_DEFAULT;
                found = 1;
            } else if (strcmp(hardware, "BCM2709") == 0 || strcmp(hardware, "BCM2836") == 0) {
                // pi 2 hardware
                peri_base = BCM2709_PERI_BASE_DEFAULT;
                found = 1;
            }
        }
        fclose(fp);
        if (!found)
            return SETUP_NOT_RPI_FAIL;
    }

    gpio_base = peri_base + GPIO_BASE_OFFSET;

    // mmap the GPIO memory registers
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0)
        return SETUP_DEVMEM_FAIL;

    if ((gpio_mem = malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL)
        return SETUP_MALLOC_FAIL;

    if ((uint32_t)gpio_mem % PAGE_SIZE)
        gpio_mem += PAGE_SIZE - ((uint32_t)gpio_mem % PAGE_SIZE);

	   /* mmap GPIO */
    gpio_map = mmap(
      (caddr_t)gpio_mem,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED|MAP_FIXED,       // SHARED with other processes
      mem_fd,           //File to map
      gpio_base         //Offset to GPIO peripheral
   );

    if ((uint32_t)gpio_map < 0)
        return SETUP_MMAP_FAIL;

    return SETUP_OK;
}

#if 0

void clear_event_detect(int gpio)
{
    int offset = EVENT_DETECT_OFFSET + (gpio/32);
    int shift = (gpio%32);

    *(gpio_map+offset) |= (1 << shift);
    short_wait();
    *(gpio_map+offset) = 0;
}

int eventdetected(int gpio)
{
    int offset, value, bit;

    offset = EVENT_DETECT_OFFSET + (gpio/32);
    bit = (1 << (gpio%32));
    value = *(gpio_map+offset) & bit;
    if (value)
        clear_event_detect(gpio);
    return value;
}

void set_rising_event(int gpio, int enable)
{
    int offset = RISING_ED_OFFSET + (gpio/32);
    int shift = (gpio%32);

    if (enable)
        *(gpio_map+offset) |= 1 << shift;
    else
        *(gpio_map+offset) &= ~(1 << shift);
    clear_event_detect(gpio);
}

void set_falling_event(int gpio, int enable)
{
    int offset = FALLING_ED_OFFSET + (gpio/32);
    int shift = (gpio%32);

    if (enable) {
        *(gpio_map+offset) |= (1 << shift);
        *(gpio_map+offset) = (1 << shift);
    } else {
        *(gpio_map+offset) &= ~(1 << shift);
    }
    clear_event_detect(gpio);
}

void set_high_event(int gpio, int enable)
{
    int offset = HIGH_DETECT_OFFSET + (gpio/32);
    int shift = (gpio%32);

    if (enable)
        *(gpio_map+offset) |= (1 << shift);
    else
        *(gpio_map+offset) &= ~(1 << shift);
    clear_event_detect(gpio);
}

void set_low_event(int gpio, int enable)
{
    int offset = LOW_DETECT_OFFSET + (gpio/32);
    int shift = (gpio%32);

    if (enable)
        *(gpio_map+offset) |= 1 << shift;
    else
        *(gpio_map+offset) &= ~(1 << shift);
    clear_event_detect(gpio);
}

void set_pullupdn(int gpio, int pud)
{
    int clk_offset = PULLUPDNCLK_OFFSET + (gpio/32);
    int shift = (gpio%32);

    if (pud == PUD_DOWN)
        *(gpio_map+PULLUPDN_OFFSET) = (*(gpio_map+PULLUPDN_OFFSET) & ~3) | PUD_DOWN;
    else if (pud == PUD_UP)
        *(gpio_map+PULLUPDN_OFFSET) = (*(gpio_map+PULLUPDN_OFFSET) & ~3) | PUD_UP;
    else  // pud == PUD_OFF
        *(gpio_map+PULLUPDN_OFFSET) &= ~3;

    short_wait();
    *(gpio_map+clk_offset) = 1 << shift;
    short_wait();
    *(gpio_map+PULLUPDN_OFFSET) &= ~3;
    *(gpio_map+clk_offset) = 0;
}

void setup_gpio(int gpio, int direction, int pud)
{
    int offset = FSEL_OFFSET + (gpio/10);
    int shift = (gpio%10)*3;

    set_pullupdn(gpio, pud);
    if (direction == OUTPUT)
        *(gpio_map+offset) = (*(gpio_map+offset) & ~(7<<shift)) | (1<<shift);
    else  // direction == INPUT
        *(gpio_map+offset) = (*(gpio_map+offset) & ~(7<<shift));
}

// Contribution by Eric Ptak <trouch@trouch.com>
int gpio_function(int gpio)
{
    int offset = FSEL_OFFSET + (gpio/10);
    int shift = (gpio%10)*3;
    int value = *(gpio_map+offset);
    value >>= shift;
    value &= 7;
    return value; // 0=input, 1=output, 4=alt0
}

void output_gpio(int gpio, int value)
{
    int offset, shift;

    if (value) // value == HIGH
        offset = SET_OFFSET + (gpio/32);
    else       // value == LOW
       offset = CLR_OFFSET + (gpio/32);

    shift = (gpio%32);

    *(gpio_map+offset) = 1 << shift;
}

int input_gpio(int gpio)
{
   int offset, value, mask;

   offset = PINLEVEL_OFFSET + (gpio/32);
   mask = (1 << gpio%32);
   value = *(gpio_map+offset) & mask;
   return value;
}

void cleanup(void)
{
    munmap((caddr_t)gpio_map, BLOCK_SIZE);
}

#endif

/*----------------------------------------------------------------------------------------------------------------------------------------*/

#define INT        2
#define NMI        3
#define CLK        4
#define HALT       1
#define DB7        27
#define DB2        22
#define MREQ       5
#define IORQ       6
#define RD         13
#define WR         19
#define DB6        26
#define DB1        21
#define DB0        20
#define WAIT       12
#define RESET      7
#define CS_16      8
#define DB5        25
#define DB4        24
#define DB3        23
#define BUSRQ      18
#define REFSH      14
#define M1         15
#define BUSAK      16
#define SI         10
#define SO         9
#define CP         11
#define M          0


typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef char byte;
typedef char bool;

#define false 0
#define true 1

#define GPIO_CLR(pin)  *(gpio_map+CLR_OFFSET) &= 1 << pin
#define GPIO_SET(pin)  *(gpio_map+SET_OFFSET) &= 1 << pin
#define GET_GPIO(g) (((*(gpio_map+PINLEVEL_OFFSET))&(1<<g))!=0?HIGH:LOW) // 0 if LOW, (1<<g) if HIGH

#define INP_GPIO(g) *(gpio_map+((g)/10)) &= ~(7<<(((g)%10)*3))|(1<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio_map+((g)/10)) |=  (1<<(((g)%10)*3))
#define GPIO_FN(g) 0
#define SET_GPIO_ALT(g,a) *(gpio_map+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define digitalWrite(g,s) {if (s==LOW) GPIO_CLR(g); else GPIO_SET(g);}
#define pinMode(g,s) {if (s==OUTPUT) OUT_GPIO(g); else INP_GPIO(g);}
#define digitalRead(g) GET_GPIO(g)

#include <time.h>
int msleep(unsigned long milisec)
{
  struct timespec req={0};
  time_t sec=(int)(milisec/1000);
  milisec=milisec-(sec*1000);
  req.tv_sec=sec;
  req.tv_nsec=milisec*1000000L;
  while(nanosleep(&req,&req)==-1)
    continue;
  return 1;
}

#define delay msleep

// Read and return one ASCII hex value from a string
byte hex(char *s){
    byte nibbleH = (*s - '0') & ~(1<<5);
    byte nibbleL = (*(s+1) - '0') & ~(1<<5);
    if (nibbleH>9) nibbleH -= 7;
    if (nibbleL>9) nibbleL -= 7;
    return (nibbleH << 4) | nibbleL;
}

// Read and return one ASCII hex value from a temp buffer given the index
// of that hex number. This is used only to read Intel HEX format buffer.
byte hexFromTemprintf(char *pTemp, int index)
{
    int start = (index*2)+1;
    return hex(pTemp + start);
}

char extraInfo[64] = { "" };

// Buffer containing RAM memory for Z80 to access
byte ram[256*256];
#define TEMP_SIZE   512
char temp[TEMP_SIZE];


int  halt;
int  mreq;
int  iorq;
int  rfsh;
int  m1;
int  busak;
int  wr;
int  rd;

// Control *input* pins of Z80, we write them into the dongle
int zint = 1;
int nmi = 1;
int reset = 1;
int busrq = 1;
int wait = 1;

// Content of address and data wires
unsigned short ab;
byte db;

// Clock counter after reset
int clkCount;
int clkCountHi;

// T-cycle counter
int T;
int Mlast;

// M1-cycle counter
int m1Count; 

// Detection if the address or data bus is tri-stated
bool abTristated = false;
bool dbTristated = false;

// Simulation control variables
bool running = 1;           // Simulation is running or is stopped
int traceShowBothPhases;    // Show both phases of a clock cycle
int traceRefresh;           // Trace refresh cycles
int tracePause;             // Pause for a key press every so many clocks
int tracePauseCount;        // Current clock count for tracePause
int stopAtClk;              // Stop the simulation after this many clocks
int stopAtM1;               // Stop at a specific M1 cycle number
int stopAtHalt;             // Stop when HALT signal gets active
int intAtClk;               // Issue INT signal at that clock number
int nmiAtClk;               // Issue NMI signal at that clock number
int busrqAtClk;             // Issue BUSRQ signal at that clock number
int resetAtClk;             // Issue RESET signal at that clock number
int waitAtClk;              // Issue WAIT signal at that clock number
int clearAtClk;             // Clear all control signals at that clock number
byte iorqVector;            // Push IORQ vector (default is FF)

void printButton(int g)
{
  if (GET_GPIO(g)) // !=0 <-> bit is 1 <- port is HIGH=3.3V
    printf("Button pressed!\n");
  else // port is LOW=0V
    printf("Button released!\n");
}

// Resets all simulation variables to their defaults
void ResetSimulationVars()
{
    traceShowBothPhases = 0;// Show both phases of a clock cycle
    traceRefresh = 0;       // Trace refresh cycles
    tracePause = -1;        // Pause for a keypress every so many clocks
    stopAtClk = -1;         // Stop the simulation after this many clocks
    stopAtM1 = -1;          // Stop at a specific M1 cycle number
    stopAtHalt = 1;         // Stop when HALT signal gets active
    intAtClk = -1;          // Issue INT signal at that clock number
    nmiAtClk = -1;          // Issue NMI signal at that clock number
    busrqAtClk = -1;        // Issue BUSRQ signal at that clock number
    resetAtClk = -1;        // Issue RESET signal at that clock number
    waitAtClk = -1;         // Issue WAIT signal at that clock number
    clearAtClk = -1;        // Clear all control signals at that clock number
    iorqVector = 0xFF;      // Push IORQ vector
}

// Issue a RESET sequence to Z80 and reset internal counters
void DoReset()
{
	int i;
    printf("\r\n:Starting the clock\r\n");
    digitalWrite(RESET, LOW);    delay(10);
    // Reset should be kept low for 3 full clock cycles
    for( i=0; i<3; i++)
    {
        digitalWrite(CLK, HIGH); delay(1);
        digitalWrite(CLK, LOW);  delay(1);
    }
    printf(":Releasing RESET\r\n");
    digitalWrite(RESET, HIGH);   delay(1);
    // Do not count initial 2 clocks after the reset
    clkCount = -2;
    T = 0;
    Mlast = 1;
    tracePauseCount = 0;
    m1Count = 0;
	digitalWrite(RESET, HIGH); 
}

// Write all control pins into the Z80 dongle
void WriteControlPins()
{
    digitalWrite(INT, zint ? HIGH : LOW);
    digitalWrite(NMI, nmi ? HIGH : LOW);
    digitalWrite(RESET, reset ? HIGH : LOW);
    digitalWrite(BUSRQ, busrq ? HIGH : LOW);
    digitalWrite(WAIT,  wait ? HIGH : LOW);
}

// Set new data value into the Z80 data bus
void SetDataToDB(byte data)
{
    pinMode(DB0, OUTPUT);
    pinMode(DB1, OUTPUT);
    pinMode(DB2, OUTPUT);
    pinMode(DB3, OUTPUT);
    pinMode(DB4, OUTPUT);
    pinMode(DB5, OUTPUT);
    pinMode(DB6, OUTPUT);
    pinMode(DB7, OUTPUT);

    digitalWrite(DB0, (data & (1<<0)) ? HIGH : LOW);
    digitalWrite(DB1, (data & (1<<1)) ? HIGH : LOW);
    digitalWrite(DB2, (data & (1<<2)) ? HIGH : LOW);
    digitalWrite(DB3, (data & (1<<3)) ? HIGH : LOW);
    digitalWrite(DB4, (data & (1<<4)) ? HIGH : LOW);
    digitalWrite(DB5, (data & (1<<5)) ? HIGH : LOW);
    digitalWrite(DB6, (data & (1<<6)) ? HIGH : LOW);
    digitalWrite(DB7, (data & (1<<7)) ? HIGH : LOW);
    db = data;
    dbTristated = false;
}

// Read Z80 data bus and store into db variable
void GetDataFromDB()
{
    pinMode(DB0, INPUT);
    pinMode(DB1, INPUT);
    pinMode(DB2, INPUT);
    pinMode(DB3, INPUT);
    pinMode(DB4, INPUT);
    pinMode(DB5, INPUT);
    pinMode(DB6, INPUT);
    pinMode(DB7, INPUT);

    digitalWrite(DB0, LOW);
    digitalWrite(DB1, LOW);
    digitalWrite(DB2, LOW);
    digitalWrite(DB3, LOW);
    digitalWrite(DB4, LOW);
    digitalWrite(DB5, LOW);
    digitalWrite(DB6, LOW);
    digitalWrite(DB7, LOW);

    // Detect if the data bus is tri-stated
    delay(1);
//    int test0 = analogRead(DB0);
    // These numbers might need to be adjusted for each Arduino board
    //dbTristated = test0>HI_Z_LOW && test0<HI_Z_HIGH;

    byte d0 = digitalRead(DB0);
    byte d1 = digitalRead(DB1);
    byte d2 = digitalRead(DB2);
    byte d3 = digitalRead(DB3);
    byte d4 = digitalRead(DB4);
    byte d5 = digitalRead(DB5);
    byte d6 = digitalRead(DB6);
    byte d7 = digitalRead(DB7);
    db = (d7<<7)|(d6<<6)|(d5<<5)|(d4<<4)|(d3<<3)|(d2<<2)|(d1<<1)|d0;
}

// Read a value of Z80 address bus and store it into the ab variable.
// In addition, try to detect when a bus is tri-stated and write 0xFFF if so.
void GetAddressFromAB()
{
	int i;
    int x = 0;
	digitalWrite(M, HIGH);
	digitalWrite(CS_16, LOW);
	digitalWrite(CP, LOW);
	digitalWrite(M, LOW);
	for(i=0;i<16;i++)
	{
		digitalWrite(CP, LOW);
		digitalWrite(CP, LOW);
		digitalWrite(CP, LOW);
		x = (x << 1) | (digitalRead(SO)) ;
		printf("%d", digitalRead(SO));
		digitalWrite(CP, HIGH);
	}
	digitalWrite(CP, LOW);
	digitalWrite(CS_16, HIGH);
	digitalWrite(CP, HIGH);
	ab = x;
}

// Read all control pins on the Z80 and store them into internal variables
void ReadControlState()
{
    halt  = digitalRead(HALT);
    mreq  = digitalRead(MREQ);
    iorq  = digitalRead(IORQ);
    rfsh  = digitalRead(REFSH);
    m1    = digitalRead(M1);
    busak = digitalRead(BUSAK);
    wr    = digitalRead(WR);
    rd    = digitalRead(RD);
}

// Dump the Z80 state as stored in internal variables
void DumpState(bool suppress)
{
    if (!suppress)
    {
        // Select your character for tri-stated bus
        char abStr[4] = { "---" };
        char dbStr[3] = { "--" };
        if (!abTristated) sprintf(abStr, "%03X", ab);
        if (!dbTristated) sprintf(dbStr, "%02X", db);
        if (T==1 && clkCountHi)
            printf("-----------------------------------------------------------+\r\n");
        printf("#%03d%c T%-2d AB:%s DB:%s  %s %s %s %s %s %s %s %s |%s%s%s%s %s\r\n",
        clkCount<0? 0 : clkCount, clkCountHi ? 'H' : 'L', T,
        abStr, dbStr,
        m1?"  ":"M1", rfsh?"    ":"RFSH", mreq?"    ":"MREQ", rd?"  ":"RD", wr?"  ":"WR", iorq?"    ":"IORQ", busak?"     ":"BUSAK",halt?"    ":"HALT",
        zint?"":"[INT]", nmi?"":"[NMI]", busrq?"":"[BUSRQ]", wait?"":"[WAIT]",
        extraInfo);
    }
    extraInfo[0] = 0;
}

void loop()
{
	int i, j;
//    delay(1); GPIO_SET(CLK); delay(1);
	GPIO_SET(CLK);

    clkCountHi = 1;
    clkCount++;
    T++;
    tracePauseCount++;
    ReadControlState();
    GetAddressFromAB();
    if (Mlast==1 && m1==0)
        T = 1, m1Count++;
    Mlast = m1;
    bool suppressDump = false;
    if (!traceRefresh & !rfsh) suppressDump = true;

    // If the number of M1 cycles has been reached, skip the rest since we dont
    // want to execute this M1 phase
    if (m1Count==stopAtM1)
    {
        sprintf(extraInfo, "Number of M1 cycles reached"), running = false;
        printf("-----------------------------------------------------------+\r\n");
        goto control;
    }
    
    // If the address is tri-stated, skip checking various combinations of
    // control signals since they may also be floating and we can't detect that
    if (!abTristated)
    {
        // Simulate read from RAM
        if (!mreq && !rd)
        {
            SetDataToDB(ram[ab & 0xFFFF]);
            if (!m1)
                sprintf(extraInfo, "Opcode read from %03X -> %02X", ab, ram[ab & 0xFF]);
            else
                sprintf(extraInfo, "Memory read from %03X -> %02X", ab, ram[ab & 0xFF]);
			if (++ab % 0x1000 == 0)
				printf("%04X\n",ab);
        }
        else
        // Simulate interrupt requesting a vector
        if (!m1 && !iorq)
        {
            SetDataToDB(iorqVector);
            sprintf(extraInfo, "Pushing vector %02X", iorqVector);
        }
        else
            GetDataFromDB();

        // Simulate write to RAM
        if (!mreq && !wr)
        {
            ram[ab & 0xFFFF] = db;
            sprintf(extraInfo, "Memory write to  %03X <- %02X", ab, db);
        }

        // Detect I/O read: We don't place anything on the bus
        if (!iorq && !rd)
        {
            sprintf(extraInfo, "I/O read from %03X", ab);
        }

        // Detect I/O write
        if (!iorq && !wr)
        {
            sprintf(extraInfo, "I/O write to %03X <- %02X", ab, db);
        }

        // Capture memory refresh cycle
        if (!mreq && !rfsh)
        {
            sprintf(extraInfo, "Refresh address  %03X", ab);
        }
    }
    else
        GetDataFromDB();

    DumpState(suppressDump);

    // If the user wanted to pause simulation after a certain number of
    // clocks, handle it here. If the key pressed to continue was not Enter,
    // stop the simulation to issue that command
    if (tracePause==tracePauseCount)
    {
        tracePauseCount = 0;
    }  

    //--------------------------------------------------------
    // Clock goes low
    //--------------------------------------------------------
    //delay(1); digitalWrite(CLK, LOW); delay(1);
	digitalWrite(CLK, LOW);
    
	clkCountHi = 0;
    if (traceShowBothPhases)
    {
        ReadControlState();
        GetAddressFromAB();
        DumpState(suppressDump);
    }

    // Perform various actions at the requested clock number
    // if the count is positive (we start it at -2 to skip initial 2T)
    if (clkCount>=0)
    {
        if (clkCount==intAtClk) zint = 0;
        if (clkCount==nmiAtClk) nmi = 0;
        if (clkCount==busrqAtClk) busrq = 0;
        if (clkCount==resetAtClk) reset = 0;
        if (clkCount==waitAtClk) wait = 0;
        // De-assert all control pins at this clock number
        if (clkCount==clearAtClk)
            zint = nmi = busrq = reset = wait = 1;
        WriteControlPins();

        // Stop the simulation under some conditions
        if (clkCount==stopAtClk)
            sprintf(extraInfo, "Number of clocks reached"), running = false;
        if (stopAtHalt&!halt)
            sprintf(extraInfo, "HALT instruction"), running = false;
    }

    //--------------------------------------------------------
    // Trace/simulation control handler
    //--------------------------------------------------------
control:    
    if (!running)
    {
        printf(":Simulation stopped: %s\r\n", extraInfo);
        extraInfo[0] = 0;
        digitalWrite(CLK, HIGH);
        zint = nmi = busrq = wait = 1;
        WriteControlPins();

        while(!running)
        {
            // Expect a command from the serial port
 //           if (Serial.available()>0)
            {
                memset((void *)temp, 0, TEMP_SIZE);
				gets(temp);
                //Serial.readBytesUntil('\r', temp, TEMP_SIZE-1);

                // Option ":"  : this is not really a user option. This is used to
                //               Intel HEX format values into the RAM buffer
                // Multiple lines may be pasted. They are separated by a space character.
                char *pTemp = temp;
                while (*pTemp==':')
                {
                    byte bytes = hexFromTemprintf(pTemp, 0);
                    if (bytes>0)
                    {
                        int address = (hexFromTemprintf(pTemp, 1)<<8) + hexFromTemprintf(pTemp, 2);
                        byte recordType = hexFromTemprintf(pTemp, 3);
                        printf("%04X:", address);
                        for (i=0; i<bytes; i++)
                        {
                            ram[(address + i) & 0xFF] = hexFromTemprintf(pTemp, 4+i);
                            printf(" %02X", hexFromTemprintf(pTemp, 4+i));
                        }
                        printf("\r\n");
                    }
                    pTemp += bytes*2 + 12;  // Skip to the next possible line of hex entry
                }
                // Option "r"  : reset and run the simulation
                if (temp[0]=='r')
                {
                    // If the variable 9 (Issue RESET) is not set, perform a RESET and run the simulation.
                    // If the variable was set, skip reset sequence since we might be testing it.
                    if (resetAtClk<0)
                        DoReset();
                    running = true;
                }
                // Option "sc" : clear simulation variables to their default values
                if (temp[0]=='s' && temp[1]=='c')
                {
                    ResetSimulationVars();
                    temp[1] = 0;            // Proceed to dump all variables...
                }
                // Option "s"  : show and set internal control variables
                if (temp[0]=='s' && temp[1]!='c')
                {
                    // Show or set the simulation parameters
                    int var = 0, value;
                    int args = sscanf(&temp[1], "%d %d\r\n", &var, &value);
                    // Parameter for the option #12 is read in as a hex; others are decimal by default
                    if (var==12)
                        args = sscanf(&temp[1], "%d %x\r\n", &var, &value);
                    if (args==2)
                    {
                        if (var==0) traceShowBothPhases = value;
                        if (var==1) traceRefresh = value;
                        if (var==2) tracePause = value;
                        if (var==3) stopAtClk = value;
                        if (var==4) stopAtM1 = value;
                        if (var==5) stopAtHalt = value;
                        if (var==6) intAtClk = value;
                        if (var==7) nmiAtClk = value;
                        if (var==8) busrqAtClk = value;
                        if (var==9) resetAtClk = value;
                        if (var==10) waitAtClk = value;
                        if (var==11) clearAtClk = value;
                        if (var==12) iorqVector = value & 0xFF;
                    }
                    printf("------ Simulation variables ------\r\n");
                    printf("#0  Trace both clock phases  = %d\r\n", traceShowBothPhases);
                    printf("#1  Trace refresh cycles     = %d\r\n", traceRefresh);
                    printf("#2  Pause for keypress every = %d\r\n", tracePause);
                    printf("#3  Stop after clock #       = %d\r\n", stopAtClk);
                    printf("#4  Stop after # M1 cycles   = %d\r\n", stopAtM1);
                    printf("#5  Stop at HALT             = %d\r\n", stopAtHalt);
                    printf("#6  Issue INT at clock #     = %d\r\n", intAtClk);
                    printf("#7  Issue NMI at clock #     = %d\r\n", nmiAtClk);
                    printf("#8  Issue BUSRQ at clock #   = %d\r\n", busrqAtClk);
                    printf("#9  Issue RESET at clock #   = %d\r\n", resetAtClk);
                    printf("#10 Issue WAIT at clock #    = %d\r\n", waitAtClk);
                    printf("#11 Clear all at clock #     = %d\r\n", clearAtClk);
                    printf("#12 Push IORQ vector #(hex)  = %2X\r\n", iorqVector);
                }
                // Option "m"  : dump RAM memory
                if (temp[0]=='m' && temp[1]!='c')
                {
                    // Dump the content of a RAM buffer
                    printf("    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n");
                    printf("   +-----------------------------------------------\r\n");
                    for(i=0; i<16; i++)
                    {
                        printf("%02X |", i);
                        for(j=0; j<16; j++)
                        {
                            printf("%02X ", ram[i*16+j]);
                        }
                        printf("\r\n");
                    }
                }
                // Option "mc"  : clear RAM memory
                if (temp[0]=='m' && temp[1]=='c')
                {
                    memset(ram, 0, sizeof(ram));
                    printf("RAM cleared\r\n");
                }
                // Option "?"  : print help
                if (temp[0]=='?' || temp[0]=='h')
                {
                    printf("s            - show simulation variables\r\n");
                    printf("s #var value - set simulation variable number to a value\r\n");
                    printf("sc           - clear simulation variables to their default values\r\n");
                    printf("r            - restart the simulation\r\n");
                    printf(":INTEL-HEX   - reload RAM buffer with a given data stream\r\n");
                    printf("m            - dump the content of the RAM buffer\r\n");
                    printf("mc           - clear the RAM buffer\r\n");
                }
            }
        }
    }	
  //return 0;
 
} // main
 

void main(void)
{
	//wiringPiSetupGpio ();
	setup();
	
	for(int i=0; i< 28; i++)
	{
		INP_GPIO(i);
		printf("%d", GET_GPIO(i));
	}
	printf("\n");
	OUT_GPIO(INT); GPIO_SET(INT);
	OUT_GPIO(NMI); GPIO_SET(NMI);
	OUT_GPIO(CLK); GPIO_SET(CLK);
	OUT_GPIO(WAIT); GPIO_SET(WAIT);
	OUT_GPIO(BUSRQ); GPIO_SET(BUSRQ);
	OUT_GPIO(RESET); GPIO_SET(RESET);
	OUT_GPIO(SI); GPIO_CLR(SI);
	OUT_GPIO(CP); GPIO_SET(CP);
	OUT_GPIO(CS_16); GPIO_SET(CS_16);
	OUT_GPIO(M);  GPIO_SET(M);
	ResetSimulationVars();  
	WriteControlPins();
	DoReset();
	for(int i=0; i< 28; i++)
	{
		printf("%d:%d\n", i, GET_GPIO(i));
	}	
#if 1
	printf("\n");
	for(int i=0; i< 28; i++)
	{
		printf("%d", GPIO_FN(i));
	}	
	printf("\n");
	GetAddressFromAB();
	printf("%04x", ab)	;
	printf("\n");
#endif
	exit(0);
	while(1) loop();
}