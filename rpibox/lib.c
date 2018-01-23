#include <stdint.h>

#include "lib.h"

/*returns the core number*/
//todo: find how much memeory is used for typedefining and convert to uint32_t
uint32_t get_core_number(void)
{
  uint32_t core_num;
  asm volatile ("mrc p15, 0, %0, c0, c0, 5" : "=r" (core_num));

  return (core_num & CORE_MASK);
}

/*start secondary core 1*/
void start_core1(void (*func)(void))
{
    store32(CORE1_STARTADDRESS, (uint32_t)func);
}

/*start secondary core 2*/
void start_core2(void (*func)(void))
{
    store32(CORE2_STARTADDRESS, (uint32_t)func);
}

/*start secondary core 3*/
void start_core3(void (*func)(void))
{
    store32(CORE3_STARTADDRESS, (uint32_t)func);
}

/*loads or reads the value from the address*/
uint32_t load32(uint32_t address)
{
	return *(uint32_t *) address;
}

/*stores or writes the value from the address*/
void store32(uint32_t address, uint32_t value)
{
	*(uint32_t *) address = value;
}

void uart_init(void)
{

}

void enable_JTAG(void)
{
 volatile uint32_t tmp;
    
 tmp = ARM_GPIO_GPFSEL2;

 tmp &= ~(7<<6); //clear the 3 bits
 tmp |= 3 << 6;  //TRST | GPIO22 (ALT4)
 
 tmp &= ~(7<<18); //clear the 3 bits
 tmp |= 3 << 18; //TDI  | GPIO26 (ALT4)
 
 tmp &= ~(7<<21); //clear the 3 bits
 tmp |= 3 << 21; //TMS  | GPIO27 (ALT4)
 
 tmp &= ~(7<<15); //clear the 3 bits
 tmp |= 3 << 15; //TCK  | GPIO25 (ALT4)
 
 tmp &= ~(7<<12); //clear the 3 bits
 tmp |= 3 << 12; //TDO  | GPIO24 (ALT4)
 
 ARM_GPIO_GPFSEL2 |= tmp;
}

//http://www.unixwiz.net/techtips/gnu-c-attributes.html#format
/* todo: enable me
#ifdef NDEBUG
#define debug_printf(...) do { } while(0)
#else
int debug_printf(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
#endif
*/