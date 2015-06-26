#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
//#include <wiringPi.h>
//#include <wiringPiSPI.h>

#include "gpio.h"

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

#define INT        2
#define NMI        3
#define CLK        4
#define HALT       17
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

#define GPIO_CLR(pin)  *(gpio_map+CLR_OFFSET) &= 1 << pin
#define GPIO_SET(pin)  *(gpio_map+SET_OFFSET) &= 1 << pin
#define GET_GPIO(g) ((*(gpio_map+PINLEVEL_OFFSET)&1<<g)!=0?HIGH:LOW) // 0 if LOW, (1<<g) if HIGH

#define INP_GPIO(g) *(gpio_map+((g)/10)) &= ~(7<<(((g)%10)*3))|(1<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio_map+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio_map+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define digitalWrite(g,s) {if (s==LOW) GPIO_CLR(g); else GPIO_SET(g);}
#define pinMode(g,s) {if (s==OUTPUT) OUT_GPIO(g); else INP_GPIO(g);}
#define digitalRead(g) GET_GPIO(g)

static volatile uint32_t *gpio_map;

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

int readAddr();

void main()
{
	int addr = 0, i = 0;;
	setup();
	pinMode(SI, OUTPUT);
	pinMode(CP, OUTPUT);
	pinMode(CS_16, OUTPUT);
	pinMode(SO, INPUT);
	digitalWrite(SI, HIGH);
	digitalWrite(CP, HIGH);
	digitalWrite(CS_16, HIGH);
	while(1)
	{
		addr = readAddr();
		for(i=0; i<16; i++)
			printf("%d", (addr & (1 << 15-i)) != 0);
		printf("\n");
	}
}

int readAddr()
{
	int i = 0, val = 0;
	pinMode(M, OUTPUT);
	digitalWrite(M, HIGH);
	digitalWrite(CS_16, LOW);
	digitalWrite(CP, LOW);
	digitalWrite(M, LOW);
	for(i=0;i<16;i++)
	{
		digitalWrite(CP, LOW);
		digitalWrite(CP, LOW);
		digitalWrite(CP, LOW);
		val = (val << 1) | digitalRead(SO);
		digitalWrite(CP, HIGH);
	}
	digitalWrite(CP, LOW);
	digitalWrite(CS_16, HIGH);
	digitalWrite(CP, HIGH);
  	return val;
}

