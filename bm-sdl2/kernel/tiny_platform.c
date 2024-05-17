#include "platform.h"

volatile struct   _dma * DMA  = (struct   _dma *) (PERIPHERAL_BASE + DMA0_BASE);
volatile struct _clock * CLK  = (struct _clock *) (PERIPHERAL_BASE + CM_BASE);
volatile struct   _pwm * PWM  = (struct   _pwm *) (PERIPHERAL_BASE + PWM_BASE);
volatile struct  _gpio * GPIO = (struct  _gpio *) (PERIPHERAL_BASE + GPIO_BASE);
volatile struct   _irq * IRQ  = (struct   _irq *) (PERIPHERAL_BASE + IRQ_BASE);

// __attribute__ ((naked)) void dmb();
// __attribute__ ((naked)) void flush_cache();

inline void dsb() {__asm("MCR p15, 0, r0, c7, c10, 4");}
inline void dmb() {__asm("MCR p15, 0, r0, c7, c10, 5");}

void mmio_write(uint32_t reg, uint32_t data) {
    dmb();
    *(volatile uint32_t *) (reg) = data;
    dmb();
}

uint32_t mmio_read(uint32_t reg) {
    dmb();
    return *(volatile uint32_t *) (reg);
    dmb();
}

void _exit(int exitcode)
{
}

void flush_cache(void)
{
    __asm(\
        "mov     r0, #0 \n "\
        "mcr     p15, #0, r0, c7, c14, #0 \n "\
        "mov     pc, lr");
}