#ifndef MEMIO_H
#define MEMIO_H

#include "lib.h"

//Raspberry Pi 2 IO base address
#if(RPI == RPI2) 
	#define ARM_IO_BASE		0x3F000000
#elif(RPI == RPI1)
	#define ARM_IO_BASE		0x20000000
#else
    #error "Please define Raspberry Pi version"
#endif

//
// General Purpose I/O
//
#define ARM_GPIO_BASE		(ARM_IO_BASE + 0x200000)

#define ARM_GPIO_GPFSEL0	 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x00))
#define ARM_GPIO_GPFSEL1	 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x04))
#define ARM_GPIO_GPFSEL2	 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x08))
#define ARM_GPIO_GPFSEL4	 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x10))
#define ARM_GPIO_GPSET0		 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x1C))
#define ARM_GPIO_GPSET1		 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x20))
#define ARM_GPIO_GPCLR0		 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x28))
#define ARM_GPIO_GPCLR1		 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x2C))
#define ARM_GPIO_GPLEV0		 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x34))
#define ARM_GPIO_GPEDS0		 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x40))
#define ARM_GPIO_GPREN0		 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x4C))
#define ARM_GPIO_GPFEN0		 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x58))
#define ARM_GPIO_GPHEN0		 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x64))
#define ARM_GPIO_GPLEN0		 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x70))
#define ARM_GPIO_GPAREN0	 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x7C))
#define ARM_GPIO_GPAFEN0	 (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x88))
#define ARM_GPIO_GPPUD           (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x94))
#define ARM_GPIO_GPPUDCLK0       (*(volatile uint32_t *)(ARM_GPIO_BASE + 0x98))

#endif //MEMIO_H