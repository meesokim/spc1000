#if defined(RASPPI)

#define DISABLE_WAIT	GPIO_SET(WAIT)
#define ENABLE_WAIT		GPIO_CLR(WAIT)
#include "rasppi.h"
#ifndef PERIPHERAL_BASE
#define PERIPHERAL_BASE ARM_IO_BASE
#endif
#define ARM_GPIO_BASE		(ARM_IO_BASE + 0x200000)
#define ARM_GPIO_GPFSEL0	(ARM_GPIO_BASE + 0x00)
#define ARM_GPIO_GPFSEL1	(ARM_GPIO_BASE + 0x04)
#define ARM_GPIO_GPFSEL2	(ARM_GPIO_BASE + 0x08)
#define ARM_GPIO_GPFSEL4	(ARM_GPIO_BASE + 0x10)
#define ARM_GPIO_GPSET0		(ARM_GPIO_BASE + 0x1C)
#define ARM_GPIO_GPCLR0		(ARM_GPIO_BASE + 0x28)
#define ARM_GPIO_GPLEV0		(ARM_GPIO_BASE + 0x34)
#define ARM_GPIO_GPEDS0		(ARM_GPIO_BASE + 0x40)
#define ARM_GPIO_GPREN0		(ARM_GPIO_BASE + 0x4C)
#define ARM_GPIO_GPFEN0		(ARM_GPIO_BASE + 0x58)
#define ARM_GPIO_GPHEN0		(ARM_GPIO_BASE + 0x64)
#define ARM_GPIO_GPLEN0		(ARM_GPIO_BASE + 0x70)
#define ARM_GPIO_GPAREN0	(ARM_GPIO_BASE + 0x7C)
#define ARM_GPIO_GPAFEN0	(ARM_GPIO_BASE + 0x88)
#define ARM_GPIO_GPPUD		(ARM_GPIO_BASE + 0x94)
#define ARM_GPIO_GPPUDCLK0	(ARM_GPIO_BASE + 0x98) 

#define GPIO_FSEL0_IN    0x0 // GPIO Function Select: GPIO Pin X0 Is An Input
#define GPIO_FSEL0_OUT   0x1 // GPIO Function Select: GPIO Pin X0 Is An Output
#define GPIO_FSEL0_ALT0  0x4 // GPIO Function Select: GPIO Pin X0 Takes Alternate Function 0
#define GPIO_FSEL0_ALT1  0x5 // GPIO Function Select: GPIO Pin X0 Takes Alternate Function 1
#define GPIO_FSEL0_ALT2  0x6 // GPIO Function Select: GPIO Pin X0 Takes Alternate Function 2
#define GPIO_FSEL0_ALT3  0x7 // GPIO Function Select: GPIO Pin X0 Takes Alternate Function 3
#define GPIO_FSEL0_ALT4  0x3 // GPIO Function Select: GPIO Pin X0 Takes Alternate Function 4
#define GPIO_FSEL0_ALT5  0x2 // GPIO Function Select: GPIO Pin X0 Takes Alternate Function 5
#define GPIO_FSEL0_CLR   0x7 // GPIO Function Select: GPIO Pin X0 Clear Bits

#define GPIO_FSEL1_IN     0x0 // GPIO Function Select: GPIO Pin X1 Is An Input
#define GPIO_FSEL1_OUT    0x8 // GPIO Function Select: GPIO Pin X1 Is An Output
#define GPIO_FSEL1_ALT0  0x20 // GPIO Function Select: GPIO Pin X1 Takes Alternate Function 0
#define GPIO_FSEL1_ALT1  0x28 // GPIO Function Select: GPIO Pin X1 Takes Alternate Function 1
#define GPIO_FSEL1_ALT2  0x30 // GPIO Function Select: GPIO Pin X1 Takes Alternate Function 2
#define GPIO_FSEL1_ALT3  0x38 // GPIO Function Select: GPIO Pin X1 Takes Alternate Function 3
#define GPIO_FSEL1_ALT4  0x18 // GPIO Function Select: GPIO Pin X1 Takes Alternate Function 4
#define GPIO_FSEL1_ALT5  0x10 // GPIO Function Select: GPIO Pin X1 Takes Alternate Function 5
#define GPIO_FSEL1_CLR   0x38 // GPIO Function Select: GPIO Pin X1 Clear Bits

#define GPIO_FSEL2_IN      0x0 // GPIO Function Select: GPIO Pin X2 Is An Input
#define GPIO_FSEL2_OUT    0x40 // GPIO Function Select: GPIO Pin X2 Is An Output
#define GPIO_FSEL2_ALT0  0x100 // GPIO Function Select: GPIO Pin X2 Takes Alternate Function 0
#define GPIO_FSEL2_ALT1  0x140 // GPIO Function Select: GPIO Pin X2 Takes Alternate Function 1
#define GPIO_FSEL2_ALT2  0x180 // GPIO Function Select: GPIO Pin X2 Takes Alternate Function 2
#define GPIO_FSEL2_ALT3  0x1C0 // GPIO Function Select: GPIO Pin X2 Takes Alternate Function 3
#define GPIO_FSEL2_ALT4   0xC0 // GPIO Function Select: GPIO Pin X2 Takes Alternate Function 4
#define GPIO_FSEL2_ALT5   0x80 // GPIO Function Select: GPIO Pin X2 Takes Alternate Function 5
#define GPIO_FSEL2_CLR   0x1C0 // GPIO Function Select: GPIO Pin X2 Clear Bits

#define GPIO_FSEL3_IN      0x0 // GPIO Function Select: GPIO Pin X3 Is An Input
#define GPIO_FSEL3_OUT   0x200 // GPIO Function Select: GPIO Pin X3 Is An Output
#define GPIO_FSEL3_ALT0  0x800 // GPIO Function Select: GPIO Pin X3 Takes Alternate Function 0
#define GPIO_FSEL3_ALT1  0xA00 // GPIO Function Select: GPIO Pin X3 Takes Alternate Function 1
#define GPIO_FSEL3_ALT2  0xC00 // GPIO Function Select: GPIO Pin X3 Takes Alternate Function 2
#define GPIO_FSEL3_ALT3  0xE00 // GPIO Function Select: GPIO Pin X3 Takes Alternate Function 3
#define GPIO_FSEL3_ALT4  0x600 // GPIO Function Select: GPIO Pin X3 Takes Alternate Function 4
#define GPIO_FSEL3_ALT5  0x400 // GPIO Function Select: GPIO Pin X3 Takes Alternate Function 5
#define GPIO_FSEL3_CLR   0xE00 // GPIO Function Select: GPIO Pin X3 Clear Bits

#define GPIO_FSEL4_IN       0x0 // GPIO Function Select: GPIO Pin X4 Is An Input
#define GPIO_FSEL4_OUT   0x1000 // GPIO Function Select: GPIO Pin X4 Is An Output
#define GPIO_FSEL4_ALT0  0x4000 // GPIO Function Select: GPIO Pin X4 Takes Alternate Function 0
#define GPIO_FSEL4_ALT1  0x5000 // GPIO Function Select: GPIO Pin X4 Takes Alternate Function 1
#define GPIO_FSEL4_ALT2  0x6000 // GPIO Function Select: GPIO Pin X4 Takes Alternate Function 2
#define GPIO_FSEL4_ALT3  0x7000 // GPIO Function Select: GPIO Pin X4 Takes Alternate Function 3
#define GPIO_FSEL4_ALT4  0x3000 // GPIO Function Select: GPIO Pin X4 Takes Alternate Function 4
#define GPIO_FSEL4_ALT5  0x2000 // GPIO Function Select: GPIO Pin X4 Takes Alternate Function 5
#define GPIO_FSEL4_CLR   0x7000 // GPIO Function Select: GPIO Pin X4 Clear Bits

#define GPIO_FSEL5_IN        0x0 // GPIO Function Select: GPIO Pin X5 Is An Input
#define GPIO_FSEL5_OUT    0x8000 // GPIO Function Select: GPIO Pin X5 Is An Output
#define GPIO_FSEL5_ALT0  0x20000 // GPIO Function Select: GPIO Pin X5 Takes Alternate Function 0
#define GPIO_FSEL5_ALT1  0x28000 // GPIO Function Select: GPIO Pin X5 Takes Alternate Function 1
#define GPIO_FSEL5_ALT2  0x30000 // GPIO Function Select: GPIO Pin X5 Takes Alternate Function 2
#define GPIO_FSEL5_ALT3  0x38000 // GPIO Function Select: GPIO Pin X5 Takes Alternate Function 3
#define GPIO_FSEL5_ALT4  0x18000 // GPIO Function Select: GPIO Pin X5 Takes Alternate Function 4
#define GPIO_FSEL5_ALT5  0x10000 // GPIO Function Select: GPIO Pin X5 Takes Alternate Function 5
#define GPIO_FSEL5_CLR   0x38000 // GPIO Function Select: GPIO Pin X5 Clear Bits

#define GPIO_FSEL6_IN         0x0 // GPIO Function Select: GPIO Pin X6 Is An Input
#define GPIO_FSEL6_OUT    0x40000 // GPIO Function Select: GPIO Pin X6 Is An Output
#define GPIO_FSEL6_ALT0  0x100000 // GPIO Function Select: GPIO Pin X6 Takes Alternate Function 0
#define GPIO_FSEL6_ALT1  0x140000 // GPIO Function Select: GPIO Pin X6 Takes Alternate Function 1
#define GPIO_FSEL6_ALT2  0x180000 // GPIO Function Select: GPIO Pin X6 Takes Alternate Function 2
#define GPIO_FSEL6_ALT3  0x1C0000 // GPIO Function Select: GPIO Pin X6 Takes Alternate Function 3
#define GPIO_FSEL6_ALT4   0xC0000 // GPIO Function Select: GPIO Pin X6 Takes Alternate Function 4
#define GPIO_FSEL6_ALT5   0x80000 // GPIO Function Select: GPIO Pin X6 Takes Alternate Function 5
#define GPIO_FSEL6_CLR   0x1C0000 // GPIO Function Select: GPIO Pin X6 Clear Bits

#define GPIO_FSEL7_IN         0x0 // GPIO Function Select: GPIO Pin X7 Is An Input
#define GPIO_FSEL7_OUT   0x200000 // GPIO Function Select: GPIO Pin X7 Is An Output
#define GPIO_FSEL7_ALT0  0x800000 // GPIO Function Select: GPIO Pin X7 Takes Alternate Function 0
#define GPIO_FSEL7_ALT1  0xA00000 // GPIO Function Select: GPIO Pin X7 Takes Alternate Function 1
#define GPIO_FSEL7_ALT2  0xC00000 // GPIO Function Select: GPIO Pin X7 Takes Alternate Function 2
#define GPIO_FSEL7_ALT3  0xE00000 // GPIO Function Select: GPIO Pin X7 Takes Alternate Function 3
#define GPIO_FSEL7_ALT4  0x600000 // GPIO Function Select: GPIO Pin X7 Takes Alternate Function 4
#define GPIO_FSEL7_ALT5  0x400000 // GPIO Function Select: GPIO Pin X7 Takes Alternate Function 5
#define GPIO_FSEL7_CLR   0xE00000 // GPIO Function Select: GPIO Pin X7 Clear Bits

#define GPIO_FSEL8_IN          0x0 // GPIO Function Select: GPIO Pin X8 Is An Input
#define GPIO_FSEL8_OUT   0x1000000 // GPIO Function Select: GPIO Pin X8 Is An Output
#define GPIO_FSEL8_ALT0  0x4000000 // GPIO Function Select: GPIO Pin X8 Takes Alternate Function 0
#define GPIO_FSEL8_ALT1  0x5000000 // GPIO Function Select: GPIO Pin X8 Takes Alternate Function 1
#define GPIO_FSEL8_ALT2  0x6000000 // GPIO Function Select: GPIO Pin X8 Takes Alternate Function 2
#define GPIO_FSEL8_ALT3  0x7000000 // GPIO Function Select: GPIO Pin X8 Takes Alternate Function 3
#define GPIO_FSEL8_ALT4  0x3000000 // GPIO Function Select: GPIO Pin X8 Takes Alternate Function 4
#define GPIO_FSEL8_ALT5  0x2000000 // GPIO Function Select: GPIO Pin X8 Takes Alternate Function 5
#define GPIO_FSEL8_CLR   0x7000000 // GPIO Function Select: GPIO Pin X8 Clear Bits

#define GPIO_FSEL9_IN           0x0 // GPIO Function Select: GPIO Pin X9 Is An Input
#define GPIO_FSEL9_OUT    0x8000000 // GPIO Function Select: GPIO Pin X9 Is An Output
#define GPIO_FSEL9_ALT0  0x20000000 // GPIO Function Select: GPIO Pin X9 Takes Alternate Function 0
#define GPIO_FSEL9_ALT1  0x28000000 // GPIO Function Select: GPIO Pin X9 Takes Alternate Function 1
#define GPIO_FSEL9_ALT2  0x30000000 // GPIO Function Select: GPIO Pin X9 Takes Alternate Function 2
#define GPIO_FSEL9_ALT3  0x38000000 // GPIO Function Select: GPIO Pin X9 Takes Alternate Function 3
#define GPIO_FSEL9_ALT4  0x18000000 // GPIO Function Select: GPIO Pin X9 Takes Alternate Function 4
#define GPIO_FSEL9_ALT5  0x10000000 // GPIO Function Select: GPIO Pin X9 Takes Alternate Function 5
#define GPIO_FSEL9_CLR   0x38000000 // GPIO Function Select: GPIO Pin X9 Clear Bits

#define ARM_IC_BASE     (ARM_IO_BASE + 0xB000)

#define ARM_IC_IRQ_BASIC_PENDING  (ARM_IC_BASE + 0x200)
#define ARM_IC_IRQ_PENDING_1      (ARM_IC_BASE + 0x204)
#define ARM_IC_IRQ_PENDING_2      (ARM_IC_BASE + 0x208)
#define ARM_IC_FIQ_CONTROL    	  (ARM_IC_BASE + 0x20C)
#define ARM_IC_ENABLE_IRQS_1      (ARM_IC_BASE + 0x210)
#define ARM_IC_ENABLE_IRQS_2      (ARM_IC_BASE + 0x214)
#define ARM_IC_ENABLE_BASIC_IRQS  (ARM_IC_BASE + 0x218)
#define ARM_IC_DISABLE_IRQS_1     (ARM_IC_BASE + 0x21C)
#define ARM_IC_DISABLE_IRQS_2     (ARM_IC_BASE + 0x220)
#define ARM_IC_DISABLE_BASIC_IRQS (ARM_IC_BASE + 0x224)

#define ARM_IRQS_PER_REG    32

#define ARM_IRQ1_BASE       0
#define ARM_IRQ2_BASE       (ARM_IRQ1_BASE + ARM_IRQS_PER_REG)
#define ARM_IRQBASIC_BASE   (ARM_IRQ2_BASE + ARM_IRQS_PER_REG)

#define ARM_IRQ_TIMER3      (ARM_IRQ1_BASE + 3)
#define ARM_IRQ_USB         (ARM_IRQ1_BASE + 9)


#define ARM_IC_IRQ_PENDING(irq) (  (irq) < ARM_IRQ2_BASE    \
                 ? ARM_IC_IRQ_PENDING_1     \
                 : ((irq) < ARM_IRQBASIC_BASE   \
                   ? ARM_IC_IRQ_PENDING_2   \
                   : ARM_IC_IRQ_BASIC_PENDING))
#define ARM_IC_IRQS_ENABLE(irq) (  (irq) < ARM_IRQ2_BASE    \
                 ? ARM_IC_ENABLE_IRQS_1     \
                 : ((irq) < ARM_IRQBASIC_BASE   \
                   ? ARM_IC_ENABLE_IRQS_2   \
                   : ARM_IC_ENABLE_BASIC_IRQS))
#define ARM_IC_IRQS_DISABLE(irq) (  (irq) < ARM_IRQ2_BASE   \
                 ? ARM_IC_DISABLE_IRQS_1    \
                 : ((irq) < ARM_IRQBASIC_BASE   \
                   ? ARM_IC_DISABLE_IRQS_2  \
                   : ARM_IC_DISABLE_BASIC_IRQS))
#define ARM_IRQ_MASK(irq)   (1 << ((irq) & (ARM_IRQS_PER_REG-1)))

#define ARM_FIQ_GPIO0		49
#define ARM_FIQ_GPIO3		52

#define BCM2835_GPIO_PADS               	(ARM_IO_BASE-0x100000)
#define BCM2835_PADS_GPIO_0_27               0x002c 
#define BCM2835_PADS_GPIO_28_45              0x0030 
#define BCM2835_PADS_GPIO_46_53              0x0034 
#define BCM2835_PAD_PASSWRD                  (0x5A << 24)  
#define BCM2835_PAD_SLEW_RATE_UNLIMITED      0x10 
#define BCM2835_PAD_HYSTERESIS_ENABLED       0x08 
#define BCM2835_PAD_DRIVE_2mA                0x00 
#define BCM2835_PAD_DRIVE_4mA                0x01 
#define BCM2835_PAD_DRIVE_6mA                0x02 
#define BCM2835_PAD_DRIVE_8mA                0x03 
#define BCM2835_PAD_DRIVE_10mA               0x04 
#define BCM2835_PAD_DRIVE_12mA               0x05 
#define BCM2835_PAD_DRIVE_14mA               0x06 
#define BCM2835_PAD_DRIVE_16mA               0x07 

#define CM_BASE   0x101000 // Clock Manager Base Address
#define CM_GNRICCTL  0x000 // Clock Manager Generic Clock Control
#define CM_GNRICDIV  0x004 // Clock Manager Generic Clock Divisor
#define CM_VPUCTL    0x008 // Clock Manager VPU Clock Control
#define CM_VPUDIV    0x00C // Clock Manager VPU Clock Divisor
#define CM_SYSCTL    0x010 // Clock Manager System Clock Control
#define CM_SYSDIV    0x014 // Clock Manager System Clock Divisor
#define CM_PERIACTL  0x018 // Clock Manager PERIA Clock Control
#define CM_PERIADIV  0x01C // Clock Manager PERIA Clock Divisor
#define CM_PERIICTL  0x020 // Clock Manager PERII Clock Control
#define CM_PERIIDIV  0x024 // Clock Manager PERII Clock Divisor
#define CM_H264CTL   0x028 // Clock Manager H264 Clock Control
#define CM_H264DIV   0x02C // Clock Manager H264 Clock Divisor
#define CM_ISPCTL    0x030 // Clock Manager ISP Clock Control
#define CM_ISPDIV    0x034 // Clock Manager ISP Clock Divisor
#define CM_V3DCTL    0x038 // Clock Manager V3D Clock Control
#define CM_V3DDIV    0x03C // Clock Manager V3D Clock Divisor
#define CM_CAM0CTL   0x040 // Clock Manager Camera 0 Clock Control
#define CM_CAM0DIV   0x044 // Clock Manager Camera 0 Clock Divisor
#define CM_CAM1CTL   0x048 // Clock Manager Camera 1 Clock Control
#define CM_CAM1DIV   0x04C // Clock Manager Camera 1 Clock Divisor
#define CM_CCP2CTL   0x050 // Clock Manager CCP2 Clock Control
#define CM_CCP2DIV   0x054 // Clock Manager CCP2 Clock Divisor
#define CM_DSI0ECTL  0x058 // Clock Manager DSI0E Clock Control
#define CM_DSI0EDIV  0x05C // Clock Manager DSI0E Clock Divisor
#define CM_DSI0PCTL  0x060 // Clock Manager DSI0P Clock Control
#define CM_DSI0PDIV  0x064 // Clock Manager DSI0P Clock Divisor
#define CM_DPICTL    0x068 // Clock Manager DPI Clock Control
#define CM_DPIDIV    0x06C // Clock Manager DPI Clock Divisor
#define CM_GP0CTL    0x070 // Clock Manager General Purpose 0 Clock Control
#define CM_GP0DIV    0x074 // Clock Manager General Purpose 0 Clock Divisor
#define CM_GP1CTL    0x078 // Clock Manager General Purpose 1 Clock Control
#define CM_GP1DIV    0x07C // Clock Manager General Purpose 1 Clock Divisor
#define CM_GP2CTL    0x080 // Clock Manager General Purpose 2 Clock Control
#define CM_GP2DIV    0x084 // Clock Manager General Purpose 2 Clock Divisor
#define CM_HSMCTL    0x088 // Clock Manager HSM Clock Control
#define CM_HSMDIV    0x08C // Clock Manager HSM Clock Divisor
#define CM_OTPCTL    0x090 // Clock Manager OTP Clock Control
#define CM_OTPDIV    0x094 // Clock Manager OTP Clock Divisor
#define CM_PCMCTL    0x098 // Clock Manager PCM / I2S Clock Control
#define CM_PCMDIV    0x09C // Clock Manager PCM / I2S Clock Divisor
#define CM_PWMCTL    0x0A0 // Clock Manager PWM Clock Control
#define CM_PWMDIV    0x0A4 // Clock Manager PWM Clock Divisor
#define CM_SLIMCTL   0x0A8 // Clock Manager SLIM Clock Control
#define CM_SLIMDIV   0x0AC // Clock Manager SLIM Clock Divisor
#define CM_SMICTL    0x0B0 // Clock Manager SMI Clock Control
#define CM_SMIDIV    0x0B4 // Clock Manager SMI Clock Divisor
#define CM_TCNTCTL   0x0C0 // Clock Manager TCNT Clock Control
#define CM_TCNTDIV   0x0C4 // Clock Manager TCNT Clock Divisor
#define CM_TECCTL    0x0C8 // Clock Manager TEC Clock Control
#define CM_TECDIV    0x0CC // Clock Manager TEC Clock Divisor
#define CM_TD0CTL    0x0D0 // Clock Manager TD0 Clock Control
#define CM_TD0DIV    0x0D4 // Clock Manager TD0 Clock Divisor
#define CM_TD1CTL    0x0D8 // Clock Manager TD1 Clock Control
#define CM_TD1DIV    0x0DC // Clock Manager TD1 Clock Divisor
#define CM_TSENSCTL  0x0E0 // Clock Manager TSENS Clock Control
#define CM_TSENSDIV  0x0E4 // Clock Manager TSENS Clock Divisor
#define CM_TIMERCTL  0x0E8 // Clock Manager Timer Clock Control
#define CM_TIMERDIV  0x0EC // Clock Manager Timer Clock Divisor
#define CM_UARTCTL   0x0F0 // Clock Manager UART Clock Control
#define CM_UARTDIV   0x0F4 // Clock Manager UART Clock Divisor
#define CM_VECCTL    0x0F8 // Clock Manager VEC Clock Control
#define CM_VECDIV    0x0FC // Clock Manager VEC Clock Divisor
#define CM_OSCCOUNT  0x100 // Clock Manager Oscillator Count
#define CM_PLLA      0x104 // Clock Manager PLLA
#define CM_PLLC      0x108 // Clock Manager PLLC
#define CM_PLLD      0x10C // Clock Manager PLLD
#define CM_PLLH      0x110 // Clock Manager PLLH
#define CM_LOCK      0x114 // Clock Manager Lock
#define CM_EVENT     0x118 // Clock Manager Event
#define CM_INTEN     0x118 // Clock Manager INTEN
#define CM_DSI0HSCK  0x120 // Clock Manager DSI0HSCK
#define CM_CKSM      0x124 // Clock Manager CKSM
#define CM_OSCFREQI  0x128 // Clock Manager Oscillator Frequency Integer
#define CM_OSCFREQF  0x12C // Clock Manager Oscillator Frequency Fraction
#define CM_PLLTCTL   0x130 // Clock Manager PLLT Control
#define CM_PLLTCNT0  0x134 // Clock Manager PLLT0
#define CM_PLLTCNT1  0x138 // Clock Manager PLLT1
#define CM_PLLTCNT2  0x13C // Clock Manager PLLT2
#define CM_PLLTCNT3  0x140 // Clock Manager PLLT3
#define CM_TDCLKEN   0x144 // Clock Manager TD Clock Enable
#define CM_BURSTCTL  0x148 // Clock Manager Burst Control
#define CM_BURSTCNT  0x14C // Clock Manager Burst
#define CM_DSI1ECTL  0x158 // Clock Manager DSI1E Clock Control
#define CM_DSI1EDIV  0x15C // Clock Manager DSI1E Clock Divisor
#define CM_DSI1PCTL  0x160 // Clock Manager DSI1P Clock Control
#define CM_DSI1PDIV  0x164 // Clock Manager DSI1P Clock Divisor
#define CM_DFTCTL    0x168 // Clock Manager DFT Clock Control
#define CM_DFTDIV    0x16C // Clock Manager DFT Clock Divisor
#define CM_PLLB      0x170 // Clock Manager PLLB
#define CM_PULSECTL  0x190 // Clock Manager Pulse Clock Control
#define CM_PULSEDIV  0x194 // Clock Manager Pulse Clock Divisor
#define CM_SDCCTL    0x1A8 // Clock Manager SDC Clock Control
#define CM_SDCDIV    0x1AC // Clock Manager SDC Clock Divisor
#define CM_ARMCTL    0x1B0 // Clock Manager ARM Clock Control
#define CM_ARMDIV    0x1B4 // Clock Manager ARM Clock Divisor
#define CM_AVEOCTL   0x1B8 // Clock Manager AVEO Clock Control
#define CM_AVEODIV   0x1BC // Clock Manager AVEO Clock Divisor
#define CM_EMMCCTL   0x1C0 // Clock Manager EMMC Clock Control
#define CM_EMMCDIV   0x1C4 // Clock Manager EMMC Clock Divisor

#define CM_SRC_OSCILLATOR        0x01 // Clock Control: Clock Source   Oscillator
#define CM_SRC_TESTDEBUG0        0x02 // Clock Control: Clock Source   Test Debug 0
#define CM_SRC_TESTDEBUG1        0x03 // Clock Control: Clock Source   Test Debug 1
#define CM_SRC_PLLAPER           0x04 // Clock Control: Clock Source   PLLA Per
#define CM_SRC_PLLCPER           0x05 // Clock Control: Clock Source   PLLC Per
#define CM_SRC_PLLDPER           0x06 // Clock Control: Clock Source   PLLD Per
#define CM_SRC_HDMIAUX           0x07 // Clock Control: Clock Source   HDMI Auxiliary
#define CM_SRC_GND               0x08 // Clock Control: Clock Source   GND
#define CM_ENAB                  0x10 // Clock Control: Enable The Clock Generator
#define CM_KILL                  0x20 // Clock Control: Kill The Clock Generator
#define CM_BUSY                  0x80 // Clock Control: Clock Generator Is Running
#define CM_FLIP                 0x100 // Clock Control: Invert The Clock Generator Output
#define CM_MASH_1               0x200 // Clock Control: MASH Control   1-Stage MASH (Equivalent To Non-MASH Dividers)
#define CM_MASH_2               0x400 // Clock Control: MASH Control   2-Stage MASH
#define CM_MASH_3               0x600 // Clock Control: MASH Control   3-Stage MASH
#define CM_PASSWORD        0x5A000000 // Clock Control: Password "5A"

/* PWM / Pulse Width Modulator Interface */

#define PWM_BASE  0x20C000 // PWM Base Address
#define PWM_CTL        0x0 // PWM Control
#define PWM_STA        0x4 // PWM Status
#define PWM_DMAC       0x8 // PWM DMA Configuration
#define PWM_RNG1      0x10 // PWM Channel 1 Range
#define PWM_DAT1      0x14 // PWM Channel 1 Data
#define PWM_FIF1      0x18 // PWM FIFO Input
#define PWM_RNG2      0x20 // PWM Channel 2 Range
#define PWM_DAT2      0x24 // PWM Channel 2 Data

#define PWM_PWEN1     0x1 // PWM Control: Channel 1 Enable
#define PWM_MODE1     0x2 // PWM Control: Channel 1 Mode
#define PWM_RPTL1     0x4 // PWM Control: Channel 1 Repeat Last Data
#define PWM_SBIT1     0x8 // PWM Control: Channel 1 Silence Bit
#define PWM_POLA1    0x10 // PWM Control: Channel 1 Polarity
#define PWM_USEF1    0x20 // PWM Control: Channel 1 Use Fifo
#define PWM_CLRF1    0x40 // PWM Control: Clear Fifo
#define PWM_MSEN1    0x80 // PWM Control: Channel 1 M/S Enable
#define PWM_PWEN2   0x100 // PWM Control: Channel 2 Enable
#define PWM_MODE2   0x200 // PWM Control: Channel 2 Mode
#define PWM_RPTL2   0x400 // PWM Control: Channel 2 Repeat Last Data
#define PWM_SBIT2   0x800 // PWM Control: Channel 2 Silence Bit
#define PWM_POLA2  0x1000 // PWM Control: Channel 2 Polarity
#define PWM_USEF2  0x2000 // PWM Control: Channel 2 Use Fifo
#define PWM_MSEN2  0x8000 // PWM Control: Channel 2 M/S Enable

#define PWM_FULL1     0x1 // PWM Status: Fifo Full Flag
#define PWM_EMPT1     0x2 // PWM Status: Fifo Empty Flag
#define PWM_WERR1     0x4 // PWM Status: Fifo Write Error Flag
#define PWM_RERR1     0x8 // PWM Status: Fifo Read Error Flag
#define PWM_GAPO1    0x10 // PWM Status: Channel 1 Gap Occurred Flag
#define PWM_GAPO2    0x20 // PWM Status: Channel 2 Gap Occurred Flag
#define PWM_GAPO3    0x40 // PWM Status: Channel 3 Gap Occurred Flag
#define PWM_GAPO4    0x80 // PWM Status: Channel 4 Gap Occurred Flag
#define PWM_BERR    0x100 // PWM Status: Bus Error Flag
#define PWM_STA1    0x200 // PWM Status: Channel 1 State
#define PWM_STA2    0x400 // PWM Status: Channel 2 State
#define PWM_STA3    0x800 // PWM Status: Channel 3 State
#define PWM_STA4   0x1000 // PWM Status: Channel 4 State

#define PWM_ENAB  0x80000000 // PWM DMA Configuration: DMA Enable

//
// System Timers
//
#define ARM_SYSTIMER_BASE	(ARM_IO_BASE + 0x3000)

#define ARM_SYSTIMER_CS		(ARM_SYSTIMER_BASE + 0x00)
#define ARM_SYSTIMER_CLO	(ARM_SYSTIMER_BASE + 0x04)
#define ARM_SYSTIMER_CHI	(ARM_SYSTIMER_BASE + 0x08)
#define ARM_SYSTIMER_C0		(ARM_SYSTIMER_BASE + 0x0C)
#define ARM_SYSTIMER_C1		(ARM_SYSTIMER_BASE + 0x10)
#define ARM_SYSTIMER_C2		(ARM_SYSTIMER_BASE + 0x14)
#define ARM_SYSTIMER_C3		(ARM_SYSTIMER_BASE + 0x18)

#define CHANS			2			// 2 I2S stereo channels
#define CHANLEN			32			// width of a channel slot in bits

//
// PCM / I2S registers
//
#define CS_A_STBY		(1 << 25)
#define CS_A_SYNC		(1 << 24)
#define CS_A_TXE		(1 << 21)
#define CS_A_TXD		(1 << 19)
#define CS_A_TXW		(1 << 17)
#define CS_A_TXERR		(1 << 15)
#define CS_A_TXSYNC		(1 << 13)
#define CS_A_DMAEN		(1 << 9)
#define CS_A_TXTHR__SHIFT	5
#define CS_A_RXCLR		(1 << 4)
#define CS_A_TXCLR		(1 << 3)
#define CS_A_TXON		(1 << 2)
#define CS_A_EN			(1 << 0)

#define MODE_A_CLKI		(1 << 22)
#define MODE_A_FSI		(1 << 20)
#define MODE_A_FLEN__SHIFT	10
#define MODE_A_FSLEN__SHIFT	0

#define TXC_A_CH1WEX		(1 << 31)
#define TXC_A_CH1EN		(1 << 30)
#define TXC_A_CH1POS__SHIFT	20
#define TXC_A_CH1WID__SHIFT	16
#define TXC_A_CH2WEX		(1 << 15)
#define TXC_A_CH2EN		(1 << 14)
#define TXC_A_CH2POS__SHIFT	4
#define TXC_A_CH2WID__SHIFT	0

#define DREQ_A_TX__SHIFT	8
#define DREQ_A_TX__MASK		(0x7F << 8)

#define BCM2835_AUX_IRQ			0x0000  /*!< xxx */
#define BCM2835_AUX_ENABLE		0x0004  /*!< */

#define BCM2835_AUX_ENABLE_UART1	0x01    /*!<  */
#define BCM2835_AUX_ENABLE_SPI0		0x02	/*!< SPI0 (SPI1 in the device) */
#define BCM2835_AUX_ENABLE_SPI1		0x04	/*!< SPI1 (SPI2 in the device) */

#define ARM_SPI0_BASE         (ARM_IO_BASE + 0x204000)

#define BCM2835_AUX_BASE	  (ARM_IO_BASE + 0x215000)

#define BCM2835_AUX_SPI_CNTL0		0x0000  /*!< */
#define BCM2835_AUX_SPI_CNTL1 		0x0004  /*!< */
#define BCM2835_AUX_SPI_STAT 		0x0008  /*!< */
#define BCM2835_AUX_SPI_PEEK		0x000C  /*!< Read but do not take from FF */
#define BCM2835_AUX_SPI_IO		0x0020  /*!< Write = TX, read=RX */
#define BCM2835_AUX_SPI_TXHOLD		0x0030  /*!< Write = TX keep CS, read=RX */

#define BCM2835_AUX_SPI_CLOCK_MIN	30500		/*!< 30,5kHz */
#define BCM2835_AUX_SPI_CLOCK_MAX	125000000 	/*!< 125Mhz */

#define BCM2835_AUX_SPI_CNTL0_SPEED	0xFFF00000  /*!< */
#define BCM2835_AUX_SPI_CNTL0_SPEED_MAX	0xFFF      /*!< */
#define BCM2835_AUX_SPI_CNTL0_SPEED_SHIFT 20        /*!< */

#define BCM2835_AUX_SPI_CNTL0_CS0_N     0x000C0000 /*!< CS 0 low */
#define BCM2835_AUX_SPI_CNTL0_CS1_N     0x000A0000 /*!< CS 1 low */
#define BCM2835_AUX_SPI_CNTL0_CS2_N 	0x00060000 /*!< CS 2 low */

#define BCM2835_AUX_SPI_CNTL0_POSTINPUT	0x00010000  /*!< */
#define BCM2835_AUX_SPI_CNTL0_VAR_CS	0x00008000  /*!< */
#define BCM2835_AUX_SPI_CNTL0_VAR_WIDTH	0x00004000  /*!< */
#define BCM2835_AUX_SPI_CNTL0_DOUTHOLD	0x00003000  /*!< */
#define BCM2835_AUX_SPI_CNTL0_ENABLE	0x00000800  /*!< */
#define BCM2835_AUX_SPI_CNTL0_CPHA_IN	0x00000400  /*!< */
#define BCM2835_AUX_SPI_CNTL0_CLEARFIFO	0x00000200  /*!< */
#define BCM2835_AUX_SPI_CNTL0_CPHA_OUT	0x00000100  /*!< */
#define BCM2835_AUX_SPI_CNTL0_CPOL	0x00000080  /*!< */
#define BCM2835_AUX_SPI_CNTL0_MSBF_OUT	0x00000040  /*!< */
#define BCM2835_AUX_SPI_CNTL0_SHIFTLEN	0x0000003F  /*!< */

#define BCM2835_AUX_SPI_CNTL1_CSHIGH	0x00000700  /*!< */
#define BCM2835_AUX_SPI_CNTL1_IDLE	0x00000080  /*!< */
#define BCM2835_AUX_SPI_CNTL1_TXEMPTY	0x00000040  /*!< */
#define BCM2835_AUX_SPI_CNTL1_MSBF_IN	0x00000002  /*!< */
#define BCM2835_AUX_SPI_CNTL1_KEEP_IN	0x00000001  /*!< */

#define BCM2835_AUX_SPI_STAT_TX_LVL	0xF0000000  /*!< */
#define BCM2835_AUX_SPI_STAT_RX_LVL	0x00F00000  /*!< */
#define BCM2835_AUX_SPI_STAT_TX_FULL	0x00000400  /*!< */
#define BCM2835_AUX_SPI_STAT_TX_EMPTY	0x00000200  /*!< */
#define BCM2835_AUX_SPI_STAT_RX_FULL	0x00000100  /*!< */
#define BCM2835_AUX_SPI_STAT_RX_EMPTY	0x00000080  /*!< */
#define BCM2835_AUX_SPI_STAT_BUSY	0x00000040  /*!< */
#define BCM2835_AUX_SPI_STAT_BITCOUNT	0x0000003F  /*!< */

/* Defines for SPI
   GPIO register offsets from BCM2835_SPI0_BASE. 
   Offsets into the SPI Peripheral block in bytes per 10.5 SPI Register Map
*/
#define BCM2835_SPI0_CS                      0x0000 /*!< SPI Master Control and Status */
#define BCM2835_SPI0_FIFO                    0x0004 /*!< SPI Master TX and RX FIFOs */
#define BCM2835_SPI0_CLK                     0x0008 /*!< SPI Master Clock Divider */
#define BCM2835_SPI0_DLEN                    0x000c /*!< SPI Master Data Length */
#define BCM2835_SPI0_LTOH                    0x0010 /*!< SPI LOSSI mode TOH */
#define BCM2835_SPI0_DC                      0x0014 /*!< SPI DMA DREQ Controls */

/* Register masks for SPI0_CS */
#define BCM2835_SPI0_CS_LEN_LONG             0x02000000 /*!< Enable Long data word in Lossi mode if DMA_LEN is set */
#define BCM2835_SPI0_CS_DMA_LEN              0x01000000 /*!< Enable DMA mode in Lossi mode */
#define BCM2835_SPI0_CS_CSPOL2               0x00800000 /*!< Chip Select 2 Polarity */
#define BCM2835_SPI0_CS_CSPOL1               0x00400000 /*!< Chip Select 1 Polarity */
#define BCM2835_SPI0_CS_CSPOL0               0x00200000 /*!< Chip Select 0 Polarity */
#define BCM2835_SPI0_CS_RXF                  0x00100000 /*!< RXF - RX FIFO Full */
#define BCM2835_SPI0_CS_RXR                  0x00080000 /*!< RXR RX FIFO needs Reading (full) */
#define BCM2835_SPI0_CS_TXD                  0x00040000 /*!< TXD TX FIFO can accept Data */
#define BCM2835_SPI0_CS_RXD                  0x00020000 /*!< RXD RX FIFO contains Data */
#define BCM2835_SPI0_CS_DONE                 0x00010000 /*!< Done transfer Done */
#define BCM2835_SPI0_CS_TE_EN                0x00008000 /*!< Unused */
#define BCM2835_SPI0_CS_LMONO                0x00004000 /*!< Unused */
#define BCM2835_SPI0_CS_LEN                  0x00002000 /*!< LEN LoSSI enable */
#define BCM2835_SPI0_CS_REN                  0x00001000 /*!< REN Read Enable */
#define BCM2835_SPI0_CS_ADCS                 0x00000800 /*!< ADCS Automatically Deassert Chip Select */
#define BCM2835_SPI0_CS_INTR                 0x00000400 /*!< INTR Interrupt on RXR */
#define BCM2835_SPI0_CS_INTD                 0x00000200 /*!< INTD Interrupt on Done */
#define BCM2835_SPI0_CS_DMAEN                0x00000100 /*!< DMAEN DMA Enable */
#define BCM2835_SPI0_CS_TA                   0x00000080 /*!< Transfer Active */
#define BCM2835_SPI0_CS_CSPOL                0x00000040 /*!< Chip Select Polarity */
#define BCM2835_SPI0_CS_CLEAR                0x00000030 /*!< Clear FIFO Clear RX and TX */
#define BCM2835_SPI0_CS_CLEAR_RX             0x00000020 /*!< Clear FIFO Clear RX  */
#define BCM2835_SPI0_CS_CLEAR_TX             0x00000010 /*!< Clear FIFO Clear TX  */
#define BCM2835_SPI0_CS_CPOL                 0x00000008 /*!< Clock Polarity */
#define BCM2835_SPI0_CS_CPHA                 0x00000004 /*!< Clock Phase */
#define BCM2835_SPI0_CS_CS                   0x00000003 /*!< Chip Select */

/*! \brief bcm2835SPIBitOrder SPI Bit order
  Specifies the SPI data bit ordering for bcm2835_spi_setBitOrder()
*/
typedef enum
{
    BCM2835_SPI_BIT_ORDER_LSBFIRST = 0,  /*!< LSB First */
    BCM2835_SPI_BIT_ORDER_MSBFIRST = 1   /*!< MSB First */
}bcm2835SPIBitOrder;

/*! \brief SPI Data mode
  Specify the SPI data mode to be passed to bcm2835_spi_setDataMode()
*/
typedef enum
{
    BCM2835_SPI_MODE0 = 0,  /*!< CPOL = 0, CPHA = 0 */
    BCM2835_SPI_MODE1 = 1,  /*!< CPOL = 0, CPHA = 1 */
    BCM2835_SPI_MODE2 = 2,  /*!< CPOL = 1, CPHA = 0 */
    BCM2835_SPI_MODE3 = 3   /*!< CPOL = 1, CPHA = 1 */
}bcm2835SPIMode;

/*! \brief bcm2835SPIChipSelect
  Specify the SPI chip select pin(s)
*/
typedef enum
{
    BCM2835_SPI_CS0 = 0,     /*!< Chip Select 0 */
    BCM2835_SPI_CS1 = 1,     /*!< Chip Select 1 */
    BCM2835_SPI_CS2 = 2,     /*!< Chip Select 2 (ie pins CS1 and CS2 are asserted) */
    BCM2835_SPI_CS_NONE = 3  /*!< No CS, control it yourself */
} bcm2835SPIChipSelect;

/*! \brief bcm2835SPIClockDivider
  Specifies the divider used to generate the SPI clock from the system clock.
  Figures below give the divider, clock period and clock frequency.
  Clock divided is based on nominal core clock rate of 250MHz on RPi1 and RPi2, and 400MHz on RPi3.
  It is reported that (contrary to the documentation) any even divider may used.
  The frequencies shown for each divider have been confirmed by measurement on RPi1 and RPi2.
  The system clock frequency on RPi3 is different, so the frequency you get from a given divider will be different.
  See comments in 'SPI Pins' for information about reliable SPI speeds.
  Note: it is possible to change the core clock rate of the RPi 3 back to 250MHz, by putting 
  \code
  core_freq=250
  \endcode
  in the config.txt
*/
typedef enum
{
    BCM2835_SPI_CLOCK_DIVIDER_65536 = 0,       /*!< 65536 = 3.814697260kHz on Rpi2, 6.1035156kHz on RPI3 */
    BCM2835_SPI_CLOCK_DIVIDER_32768 = 32768,   /*!< 32768 = 7.629394531kHz on Rpi2, 12.20703125kHz on RPI3 */
    BCM2835_SPI_CLOCK_DIVIDER_16384 = 16384,   /*!< 16384 = 15.25878906kHz on Rpi2, 24.4140625kHz on RPI3 */
    BCM2835_SPI_CLOCK_DIVIDER_8192  = 8192,    /*!< 8192 = 30.51757813kHz on Rpi2, 48.828125kHz on RPI3 */
    BCM2835_SPI_CLOCK_DIVIDER_4096  = 4096,    /*!< 4096 = 61.03515625kHz on Rpi2, 97.65625kHz on RPI3 */
    BCM2835_SPI_CLOCK_DIVIDER_2048  = 2048,    /*!< 2048 = 122.0703125kHz on Rpi2, 195.3125kHz on RPI3 */
    BCM2835_SPI_CLOCK_DIVIDER_1024  = 1024,    /*!< 1024 = 244.140625kHz on Rpi2, 390.625kHz on RPI3 */
    BCM2835_SPI_CLOCK_DIVIDER_512   = 512,     /*!< 512 = 488.28125kHz on Rpi2, 781.25kHz on RPI3 */
    BCM2835_SPI_CLOCK_DIVIDER_256   = 256,     /*!< 256 = 976.5625kHz on Rpi2, 1.5625MHz on RPI3 */
    BCM2835_SPI_CLOCK_DIVIDER_128   = 128,     /*!< 128 = 1.953125MHz on Rpi2, 3.125MHz on RPI3 */
    BCM2835_SPI_CLOCK_DIVIDER_64    = 64,      /*!< 64 = 3.90625MHz on Rpi2, 6.250MHz on RPI3 */
    BCM2835_SPI_CLOCK_DIVIDER_32    = 32,      /*!< 32 = 7.8125MHz on Rpi2, 12.5MHz on RPI3 */
    BCM2835_SPI_CLOCK_DIVIDER_16    = 16,      /*!< 16 = 15.625MHz on Rpi2, 25MHz on RPI3 */
    BCM2835_SPI_CLOCK_DIVIDER_8     = 8,       /*!< 8 = 31.25MHz on Rpi2, 50MHz on RPI3 */
    BCM2835_SPI_CLOCK_DIVIDER_4     = 4,       /*!< 4 = 62.5MHz on Rpi2, 100MHz on RPI3. Dont expect this speed to work reliably. */
    BCM2835_SPI_CLOCK_DIVIDER_2     = 2,       /*!< 2 = 125MHz on Rpi2, 200MHz on RPI3, fastest you can get. Dont expect this speed to work reliably.*/
    BCM2835_SPI_CLOCK_DIVIDER_1     = 1        /*!< 1 = 3.814697260kHz on Rpi2, 6.1035156kHz on RPI3, same as 0/65536 */
} bcm2835SPIClockDivider;


#define RA08	(1 << 8)
#define RA09	(1 << 9)
#define RA10	(1 << 10)
#define RA11	(1 << 11)
#define RA12	(1 << 12)
#define RA13	(1 << 13)
#define RA14	(1 << 14)
#define RA15	(1 << 15)
#define RC16	(1 << 16)
#define RC17	(1 << 17)
#define RC18	(1 << 18)
#define RC19	(1 << 19)
#define RC20	(1 << 20)
#define RC21	(1 << 21)
#define RC22	(1 << 22)
#define RC23	(1 << 23)
#define RC24	(1 << 24)
#define RC25	(1 << 25)
#define RC26	(1 << 26)
#define RC27	(1 << 27)

#define ALT0 4
#define INPUT 0
#define OUTPUT 1

#define DAT_EN	RC16
#define SLTSL	RC17
#define SNDOUT	RC18
#define IORQ	RC27
#define RD 		RC20
#define ADDR	RC22
#define INTR  	RC23
#define WAIT	RC24
#define DAT_DIR	RC25
#define MREQ	RC26
#define CLK		RA12
#define M1	    RA13
#define WR		RA14
#define RESET	RA15
#define BUSDIR  0
#define ADDRL	0

#define IOSEL0 (GPIO_FSEL0_OUT | GPIO_FSEL1_OUT | GPIO_FSEL2_OUT | GPIO_FSEL3_OUT | GPIO_FSEL4_OUT | GPIO_FSEL5_OUT | GPIO_FSEL6_OUT | GPIO_FSEL7_OUT)
#define IOSEL1 (GPIO_FSEL6_OUT | GPIO_FSEL8_ALT0 | GPIO_FSEL9_ALT0) //2 << (8 * 3))
#define IOSEL2 (GPIO_FSEL1_ALT0 | GPIO_FSEL2_OUT | GPIO_FSEL3_OUT | GPIO_FSEL4_OUT | GPIO_FSEL5_OUT)

// #ifndef uint32_t
// typedef unsigned int uint32_t;
// #endif
// #ifndef uint64_t
// typedef unsigned long uint64_t;
// #endif
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
#if 1
#define GPIO_GET() (*(volatile unsigned int*)(ARM_GPIO_GPLEV0))
#else
inline uint32_t GPIO_GET()
{
	return *(volatile unsigned int*)(ARM_GPIO_GPLEV0);
}
#endif
#define GPIO_SET(a) *(volatile unsigned int*)(ARM_GPIO_GPSET0) = a
#define GPIO_LEV(a) *(volatile unsigned int*)(ARM_GPIO_GPLEV0) = a
#define GPIO_CLR(a) *(volatile unsigned int*)(ARM_GPIO_GPCLR0) = a	
#define GPIO_SEL0(a) *(volatile unsigned int*)(ARM_GPIO_GPFSEL0) = a
#define GPIO_SEL1(a) *(volatile unsigned int*)(ARM_GPIO_GPFSEL1) = a
#define GPIO_SEL2(a) *(volatile unsigned int*)(ARM_GPIO_GPFSEL2) = a
#define PUT32(a, b) *(volatile unsigned int*)(a) = b
#define GET32(a) (*(volatile unsigned int*)(a))
#define PUT_AUX(a, b) *(volatile unsigned int*)(BCM2835_AUX_BASE + a) = b
#define GET_AUX(a) *(volatile unsigned int*)(BCM2835_AUX_BASE + a)
#define PUT_SPI(a, b) *(volatile unsigned int*)(ARM_SPI0_BASE + a) = b
#define GET_SPI(a) *(volatile unsigned int*)(ARM_SPI0_BASE + a)
#define GET64(a) (*(volatile int64_t *)(a))
#define GPPUDCKL0(a) *(volatile unsigned int*)(ARM_GPIO_GPPUDCLK0) = a
#define GPPUD(a) *(volatile unsigned int*)(ARM_GPIO_GPPUD) = a

//#define	EnableInterrupts()	__asm volatile ("cpsie i")
//#define	DisableInterrupts()	__asm volatile ("cpsid i")

typedef struct {
	volatile uint32_t IRQ_basic_pending;
	volatile uint32_t IRQ_pending_1;
	volatile uint32_t IRQ_pending_2;
	volatile uint32_t FIQ_control;
	volatile uint32_t Enable_IRQs_1;
	volatile uint32_t Enable_IRQs_2;
	volatile uint32_t Enable_Basic_IRQs;
	volatile uint32_t Disable_IRQs_1;
	volatile uint32_t Disable_IRQs_2;
	volatile uint32_t Disable_Basic_IRQs;
	} rpi_irq_controller_t;
	
/** @brief The BCM2835 Interupt controller peripheral at it's base address */
static rpi_irq_controller_t* rpiIRQController =
		(rpi_irq_controller_t*)ARM_IC_IRQ_BASIC_PENDING;	
/**
	@brief Return the IRQ Controller register set
*/
rpi_irq_controller_t* RPI_GetIrqController( void )
{
	return rpiIRQController;
}

enum TGPIOInterrupt0
{
	GPIOInterruptOnRisingEdge0,
	GPIOInterruptOnFallingEdge0,
	GPIOInterruptOnHighLevel0,
	GPIOInterruptOnLowLevel0,
	GPIOInterruptOnAsyncRisingEdge0,
	GPIOInterruptOnAsyncFallingEdge0,
	GPIOInterruptUnknown0
}; 

class CFIQPin 
{
private:
	unsigned int m_nRegOffset;
	unsigned int m_nRegMask;
	unsigned int  read32 (unsigned int nAddress)
	{
		return *(unsigned int  volatile *) nAddress;
	}
	void write32 (unsigned int nAddress, unsigned int nValue)
	{
		*(unsigned int volatile *) nAddress = nValue;
	}	
public:
	CFIQPin ()
	{
		m_nRegOffset = 0;
		m_nRegMask = 0;
	}
	void EnableIRQ(unsigned nIRQ) 
	{
		write32 (ARM_IC_IRQS_ENABLE (nIRQ), ARM_IRQ_MASK (nIRQ));   
	}
	void EnableFIQ(unsigned nFIQ) 
	{
		write32 (ARM_IC_FIQ_CONTROL, nFIQ | 0x80);
		RPI_GetIrqController()->FIQ_control = nFIQ | 0x80; 	
	}
	void AssignPin(unsigned m_nPin)
	{
		m_nRegOffset = (m_nPin / 32) * 4;
		m_nRegMask = 1 << (m_nPin % 32); 
	}
	void EnableInterrupt(TGPIOInterrupt0 Interrupt)
	{
		unsigned int nReg =  ARM_GPIO_GPREN0
				+ m_nRegOffset
				+ (Interrupt - GPIOInterruptOnRisingEdge0) * 12;
		m_nRegMask = read32 (nReg) | m_nRegMask;
		write32 (nReg, m_nRegMask);
		
	}
	inline void ResetFlag()
	{
		write32 (ARM_GPIO_GPEDS0+m_nRegOffset, m_nRegMask);
	}
};
extern void flush_cache();
extern void dmb();
extern void dsb();
extern void clean_cache();
void restore_context() {}
#else
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
void dmb() {}
void dsb() {}
void restore_context() {}
#endif
#if !defined(NO_OFFS_T)
typedef int offs_t;
#endif
#define CacheOp(x) __asm("MCR p15, 0, r0, c7, c14, "#x);
#define DSB() __asm("MCR p15, 0, r0, c7, c10, 4");
#define DMB() __asm("MCR p15, 0, r0, c7, c10, 5");
