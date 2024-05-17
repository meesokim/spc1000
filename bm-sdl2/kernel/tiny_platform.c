#include "platform.h"

volatile struct   _dma * DMA  = (struct   _dma *) (PERIPHERAL_BASE + DMA0_BASE);
volatile struct _clock * CLK  = (struct _clock *) (PERIPHERAL_BASE + CM_BASE);
volatile struct   _pwm * PWM  = (struct   _pwm *) (PERIPHERAL_BASE + PWM_BASE);
volatile struct  _gpio * GPIO = (struct  _gpio *) (PERIPHERAL_BASE + GPIO_BASE);
volatile struct   _irq * IRQ  = (struct   _irq *) (PERIPHERAL_BASE + IRQ_BASE);

void flush_cache()
{
    __asm__(
        "mov     r0, #0\n"
        "mcr     p15, #0, r0, c7, c14, #0\n"
        "mov     r0, #0\n"
        "mov     pc, lr"
    );
}
