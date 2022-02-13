#ifndef LIB_H
#define LIB_H

//#include "memio.h"
#include <circle/bcm2835.h>

#define RPI 1
#define RPI1 1
#define RPI2 2

#define CORE_MASK 3

#define CORE0 0
#define CORE1 1
#define CORE2 2
#define CORE3 3

#define CORE1_STARTADDRESS 0x4000009C
#define CORE2_STARTADDRESS 0x400000AC
#define CORE3_STARTADDRESS 0x400000BC

#define ARM_GPIO_GPFSEL2	 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x08))
//ACT LED is on GPIO47 (GPIO Function Select 4)
#define ENABLE_ACT_LED() ARM_GPIO_GPFSEL4 |= 1 << 21;

#define ACT_LED_ON()  ARM_GPIO_GPSET1 |= 1 << 15;
#define ACT_LED_OFF() ARM_GPIO_GPCLR1 |= 1 << 15;

uint32_t get_core_number(void);
void start_core1(void (*func)(void));
void start_core2(void (*func)(void));
void start_core3(void (*func)(void));

void enable_JTAG(void);

uint32_t load32(uint32_t address);
void store32(uint32_t address, uint32_t value);

#endif //LIB_H