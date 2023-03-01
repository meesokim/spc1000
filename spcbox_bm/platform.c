/*
 * Copyright (c) 2014 Marco Maccaferri and Others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "platform.h"
#include <stdarg.h>

volatile struct   _dma * DMA  = (struct   _dma *) (PERIPHERAL_BASE + DMA0_BASE);
volatile struct _clock * CLK  = (struct _clock *) (PERIPHERAL_BASE + CM_BASE);
volatile struct   _pwm * PWM  = (struct   _pwm *) (PERIPHERAL_BASE + PWM_BASE);
volatile struct   _pcm * PCM  = (struct   _pcm *) (PERIPHERAL_BASE + PCM_BASE);
volatile struct  _gpio * GPIO = (struct  _gpio *) (PERIPHERAL_BASE + GPIO_BASE);
volatile struct   _irq * IRQ  = (struct   _irq *) (PERIPHERAL_BASE + IRQ_BASE);

/* Unhandled exceptions - hang the machine */
__attribute__ ((naked)) void bad_exception() {
    while(1)
        ;
}

__attribute__ ((interrupt ("SWI"))) void interrupt_swi() {

}

__attribute__ ((interrupt ("ABORT"))) void interrupt_prefetch_abort() {

}

__attribute__ ((interrupt ("ABORT"))) void interrupt_data_abort() {

}

__attribute__((interrupt("IRQ"))) void interrupt_irq()
{
}

/*-[ARMaddrToGPUaddr]-------------------------------------------------------}
. Converts an ARM address to GPU address by using the GPU_alias offset
.--------------------------------------------------------------------------*/
uint32_t ARMaddrToGPUaddr(void *ARMaddress);

/*-[GPUaddrToARMaddr]-------------------------------------------------------}
. Converts a GPU address to an ARM address by using the GPU_alias offset
.--------------------------------------------------------------------------*/
uint32_t GPUaddrToARMaddr(uint32_t GPUaddress);

void mmio_write(uint32_t reg, uint32_t data) {
//    dmb();
    *(volatile uint32_t *) (reg) = data;
    dmb();
}

uint32_t mmio_read(uint32_t reg) {
//    dmb();
    return *(volatile uint32_t *) (reg);
    dmb();
}

void mbox_write(uint8_t channel, uint32_t data) {
    while (mmio_read(MAIL_BASE + MAIL_STATUS) & MAIL_FULL)
        ;
    mmio_write(MAIL_BASE + MAIL_WRITE, (data & 0xfffffff0) | (uint32_t) (channel & 0xf));
}

typedef enum {
	MB_CHANNEL_POWER = 0x0,								// Mailbox Channel 0: Power Management Interface 
	MB_CHANNEL_FB = 0x1,								// Mailbox Channel 1: Frame Buffer
	MB_CHANNEL_VUART = 0x2,								// Mailbox Channel 2: Virtual UART
	MB_CHANNEL_VCHIQ = 0x3,								// Mailbox Channel 3: VCHIQ Interface
	MB_CHANNEL_LEDS = 0x4,								// Mailbox Channel 4: LEDs Interface
	MB_CHANNEL_BUTTONS = 0x5,							// Mailbox Channel 5: Buttons Interface
	MB_CHANNEL_TOUCH = 0x6,								// Mailbox Channel 6: Touchscreen Interface
	MB_CHANNEL_COUNT = 0x7,								// Mailbox Channel 7: Counter
	MB_CHANNEL_TAGS = 0x8,								// Mailbox Channel 8: Tags (ARM to VC)
	MB_CHANNEL_GPU = 0x9,								// Mailbox Channel 9: GPU (VC to ARM)
} MAILBOX_CHANNEL;
/*--------------------------------------------------------------------------}
;{               RASPBERRY PI MAILBOX HARRDWARE REGISTERS					}
;{-------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) MailBoxRegisters {
	const uint32_t Read0;											// 0x00         Read data from VC to ARM
	uint32_t Unused[3];												// 0x04-0x0F
	uint32_t Peek0;													// 0x10
	uint32_t Sender0;												// 0x14
	uint32_t Status0;												// 0x18         Status of VC to ARM
	uint32_t Config0;												// 0x1C        
	uint32_t Write1;												// 0x20         Write data from ARM to VC
	uint32_t Unused2[3];											// 0x24-0x2F
	uint32_t Peek1;													// 0x30
	uint32_t Sender1;												// 0x34
	uint32_t Status1;												// 0x38         Status of ARM to VC
	uint32_t Config1;												// 0x3C 
};

#define MAILBOX ((volatile __attribute__((aligned(4))) struct MailBoxRegisters*)(uintptr_t)(PERIPHERAL_BASE + 0xB880))
/*-[mailbox_read]-----------------------------------------------------------}
. This will read any pending data on the mailbox system on the given channel.
. RETURN: The read value for success, 0xFEEDDEAD for failure.
. 04Jul17 LdB
.--------------------------------------------------------------------------*/
uint32_t mbox_read (MAILBOX_CHANNEL channel) 
{
	uint32_t value;													// Temporary read value
	if (channel > MB_CHANNEL_GPU)  return 0xFEEDDEAD;				// Channel error
	do {
		do {
			value = MAILBOX->Status0;								// Read mailbox0 status
		} while ((value & MAIL_EMPTY) != 0);						// Wait for data in mailbox
		value = MAILBOX->Read0;										// Read the mailbox	
	} while ((value & 0xF) != channel);								// We have response back
	value &= ~(0xF);												// Lower 4 low channel bits are not part of message
	return value;													// Return the value
}

uint32_t mbox_read2(uint8_t channel) {
    while (1) {
        while (mmio_read(MAIL_BASE + MAIL_STATUS) & MAIL_EMPTY)
            ;

        uint32_t data = mmio_read(MAIL_BASE + MAIL_READ);
        uint8_t read_channel = (uint8_t) (data & 0xf);
        if (read_channel == channel) {
            return (data & 0xfffffff0);
        }
    }
}


#define TIMER_CLO       0x20003004

int usleep(useconds_t usec) {
    unsigned int cur_timer = mmio_read(TIMER_CLO);
    unsigned int trigger_value = cur_timer + usec;
    unsigned int rollover;

    if (trigger_value > cur_timer)
        rollover = 0;
    else
        rollover = 1;

    for (;;) {
        cur_timer = mmio_read(TIMER_CLO);
        if (cur_timer < trigger_value) {
            if (rollover) {
                rollover = 0;
            }
        }
        else if (!rollover) {
            break;
        }
    }

    return 0;
}

void register_timer(struct timer_wait * tw, unsigned int usec) {
    unsigned int cur_timer = mmio_read(TIMER_CLO);

    tw->usec = usec;
    tw->rollover = 0;
    tw->trigger_value = 0;

    if (usec > 0) {
        tw->trigger_value = cur_timer + usec;
        if (tw->trigger_value > cur_timer)
            tw->rollover = 0;
        else
            tw->rollover = 1;
    }
}

int compare_timer(struct timer_wait * tw) {
    unsigned int cur_timer = mmio_read(TIMER_CLO);

    if (cur_timer < tw->trigger_value) {
        if (tw->rollover)
            tw->rollover = 0;
    } else if (!tw->rollover) {
        if (tw->usec > 0) {
            tw->trigger_value = cur_timer + tw->usec;
            if (tw->trigger_value > cur_timer)
                tw->rollover = 0;
            else
                tw->rollover = 1;
        }
        return 1;
    }

    return 0;
}

unsigned int sleep(unsigned int seconds) {
    usleep(seconds * 1000000);
    return 0;
}

/*--------------------------------------------------------------------------}
{   IRQ BASIC PENDING REGISTER - BCM2835.PDF Manual Section 7 page 113/114  }
{--------------------------------------------------------------------------*/
typedef union
{
    struct __attribute__((__packed__, aligned(4)))
    {
        const unsigned Timer_IRQ_pending : 1;              // @0 Timer Irq pending  ** Read only
        const unsigned Mailbox_IRQ_pending : 1;            // @1 Mailbox Irq pending  ** Read only
        const unsigned Doorbell0_IRQ_pending : 1;          // @2 Arm Doorbell0 Irq pending  ** Read only
        const unsigned Doorbell1_IRQ_pending : 1;          // @3 Arm Doorbell0 Irq pending  ** Read only
        const unsigned GPU0_halted_IRQ_pending : 1;        // @4 GPU0 halted IRQ pending  ** Read only
        const unsigned GPU1_halted_IRQ_pending : 1;        // @5 GPU1 halted IRQ pending  ** Read only
        const unsigned Illegal_access_type1_pending : 1;   // @6 Illegal access type 1 IRQ pending  ** Read only
        const unsigned Illegal_access_type0_pending : 1;   // @7 Illegal access type 0 IRQ pending  ** Read only
        const unsigned Bits_set_in_pending_register_1 : 1; // @8 One or more bits set in pending register 1  ** Read only
        const unsigned Bits_set_in_pending_register_2 : 1; // @9 One or more bits set in pending register 2  ** Read only
        const unsigned GPU_IRQ_7_pending : 1;              // @10 GPU irq 7 pending  ** Read only
        const unsigned GPU_IRQ_9_pending : 1;              // @11 GPU irq 9 pending  ** Read only
        const unsigned GPU_IRQ_10_pending : 1;             // @12 GPU irq 10 pending  ** Read only
        const unsigned GPU_IRQ_18_pending : 1;             // @13 GPU irq 18 pending  ** Read only
        const unsigned GPU_IRQ_19_pending : 1;             // @14 GPU irq 19 pending  ** Read only
        const unsigned GPU_IRQ_53_pending : 1;             // @15 GPU irq 53 pending  ** Read only
        const unsigned GPU_IRQ_54_pending : 1;             // @16 GPU irq 54 pending  ** Read only
        const unsigned GPU_IRQ_55_pending : 1;             // @17 GPU irq 55 pending  ** Read only
        const unsigned GPU_IRQ_56_pending : 1;             // @18 GPU irq 56 pending  ** Read only
        const unsigned GPU_IRQ_57_pending : 1;             // @19 GPU irq 57 pending  ** Read only
        const unsigned GPU_IRQ_62_pending : 1;             // @20 GPU irq 62 pending  ** Read only
        unsigned reserved : 10;                            // @21-31 reserved
    };
    const uint32_t Raw32; // Union to access all 32 bits as a uint32_t  ** Read only
} irq_basic_pending_reg_t;

/*==========================================================================}
{		  PUBLIC PI MAILBOX ROUTINES PROVIDED BY RPi-SmartStart API			}
{==========================================================================*/
#define MAIL_EMPTY 0x40000000 /* Mailbox Status Register: Mailbox Empty */
#define MAIL_FULL 0x80000000  /* Mailbox Status Register: Mailbox Full  */

/*-[mailbox_write]----------------------------------------------------------}
. This will execute the sending of the given data block message thru the
. mailbox system on the given channel.
. RETURN: TRUE for success, False for failure.
. 04Jul17 LdB
.--------------------------------------------------------------------------*/
#define FALSE 0
#define TRUE 1
char mailbox_write(MAILBOX_CHANNEL channel, uint32_t message)
{
    uint32_t value; // Temporary read value
    if (channel > MB_CHANNEL_GPU)
        return FALSE;   // Channel error
    message &= ~(0xF);  // Make sure 4 low channel bits are clear
    message |= channel; // OR the channel bits to the value
    do
    {
        value = MAILBOX->Status1;       // Read mailbox1 status from GPU
    } while ((value & MAIL_FULL) != 0); // Make sure arm mailbox is not full
    MAILBOX->Write1 = message;          // Write value to mailbox
    return TRUE;                        // Write success
}

/*-[mailbox_read]-----------------------------------------------------------}
. This will read any pending data on the mailbox system on the given channel.
. RETURN: The read value for success, 0xFEEDDEAD for failure.
. 04Jul17 LdB
.--------------------------------------------------------------------------*/
uint32_t mailbox_read(MAILBOX_CHANNEL channel)
{
    uint32_t value; // Temporary read value
    if (channel > MB_CHANNEL_GPU)
        return 0xFEEDDEAD; // Channel error
    do
    {
        do
        {
            value = MAILBOX->Status0;        // Read mailbox0 status
        } while ((value & MAIL_EMPTY) != 0); // Wait for data in mailbox
        value = MAILBOX->Read0;              // Read the mailbox
    } while ((value & 0xF) != channel);      // We have response back
    value &= ~(0xF);                         // Lower 4 low channel bits are not part of message
    return value;                            // Return the value
}

/*-[mailbox_tag_message]----------------------------------------------------}
. This will post and execute the given variadic data onto the tags channel
. on the mailbox system. You must provide the correct number of response
. uint32_t variables and a pointer to the response buffer. You nominate the
. number of data uint32_t for the call and fill the variadic data in. If you
. do not want the response data back the use NULL for response_buf pointer.
. RETURN: TRUE for success and the response data will be set with data
.         False for failure and the response buffer is untouched.
. 04Jul17 LdB
.--------------------------------------------------------------------------*/
char mailbox_tag_message(uint32_t *response_buf, // Pointer to response buffer
                         uint8_t data_count,     // Number of uint32_t data following
                         ...)                    // Variadic uint32_t values for call
{
    uint32_t __attribute__((aligned(16))) message[32];
    va_list list;
    va_start(list, data_count);        // Start variadic argument
    message[0] = (data_count + 3) * 4; // Size of message needed
    message[data_count + 2] = 0;       // Set end pointer to zero
    message[1] = 0;                    // Zero response message
    for (int i = 0; i < data_count; i++)
    {
        message[2 + i] = va_arg(list, uint32_t); // Fetch next variadic
    }
    va_end(list);                                                  // variadic cleanup
    mailbox_write(MB_CHANNEL_TAGS, ARMaddrToGPUaddr(&message[0])); // Write message to mailbox
    mailbox_read(MB_CHANNEL_TAGS);                                 // Wait for write response
    if (message[1] == 0x80000000)
    {
        if (response_buf)
        { // If buffer NULL used then don't want response
            for (int i = 0; i < data_count; i++)
                response_buf[i] = message[2 + i]; // Transfer out each response message
        }
        return TRUE; // message success
    }
    return FALSE; // Message failed
}
/*--------------------------------------------------------------------------}
{	ENUMERATED TIMER CONTROL PRESCALE ... BCM2835.PDF MANUAL see page 197	}
{--------------------------------------------------------------------------*/
/* In binary so any error is obvious */
typedef enum
{
    Clkdiv1 = 0b00,          // 0
    Clkdiv16 = 0b01,         // 1
    Clkdiv256 = 0b10,        // 2
    Clkdiv_undefined = 0b11, // 3
} TIMER_PRESCALE;

/*--------------------------------------------------------------------------}
{	   TIMER_CONTROL REGISTER BCM2835 ARM Peripheral manual page 197		}
{--------------------------------------------------------------------------*/
typedef union
{
    struct
    {
        unsigned unused : 1;         // @0 Unused bit
        unsigned Counter32Bit : 1;   // @1 Counter32 bit (16bit if false)
        TIMER_PRESCALE Prescale : 2; // @2-3 Prescale
        unsigned unused1 : 1;        // @4 Unused bit
        unsigned TimerIrqEnable : 1; // @5 Timer irq enable
        unsigned unused2 : 1;        // @6 Unused bit
        unsigned TimerEnable : 1;    // @7 Timer enable
        unsigned reserved : 24;      // @8-31 reserved
    };
    uint32_t Raw32; // Union to access all 32 bits as a uint32_t
} time_ctrl_reg_t;
/*--------------------------------------------------------------------------}
{	   FIQ CONTROL REGISTER BCM2835.PDF ARM Peripheral manual page 116		}
{--------------------------------------------------------------------------*/
typedef union
{
    struct __attribute__((__packed__, aligned(4)))
    {
        unsigned SelectFIQSource : 7; // @0-6 Select FIQ source
        unsigned EnableFIQ : 1;       // @7 enable FIQ
        unsigned reserved : 24;       // @8-31 reserved
    };
    uint32_t Raw32; // Union to access all 32 bits as a uint32_t
} fiq_control_reg_t;
/*--------------------------------------------------------------------------}
{	 ENABLE BASIC IRQ REGISTER BCM2835 ARM Peripheral manual page 117		}
{--------------------------------------------------------------------------*/
typedef union
{
    struct __attribute__((__packed__, aligned(4)))
    {
        unsigned Enable_Timer_IRQ : 1;            // @0 Timer Irq enable
        unsigned Enable_Mailbox_IRQ : 1;          // @1 Mailbox Irq enable
        unsigned Enable_Doorbell0_IRQ : 1;        // @2 Arm Doorbell0 Irq enable
        unsigned Enable_Doorbell1_IRQ : 1;        // @3 Arm Doorbell0 Irq enable
        unsigned Enable_GPU0_halted_IRQ : 1;      // @4 GPU0 halted IRQ enable
        unsigned Enable_GPU1_halted_IRQ : 1;      // @5 GPU1 halted IRQ enable
        unsigned Enable_Illegal_access_type1 : 1; // @6 Illegal access type 1 IRQ enable
        unsigned Enable_Illegal_access_type0 : 1; // @7 Illegal access type 0 IRQ enable
        unsigned reserved : 24;                   // @8-31 reserved
    };
    uint32_t Raw32; // Union to access all 32 bits as a uint32_t
} irq_enable_basic_reg_t;

/*--------------------------------------------------------------------------}
{   RASPBERRY PI ARM TIMER HARDWARE REGISTERS - BCM2835 Manual Section 14	}
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) ArmTimerRegisters
{
    uint32_t Load;            // 0x00
    const uint32_t Value;     // 0x04  ** Read only hence const
    time_ctrl_reg_t Control;  // 0x08
    uint32_t Clear;           // 0x0C
    const uint32_t RawIRQ;    // 0x10  ** Read only hence const
    const uint32_t MaskedIRQ; // 0x14  ** Read only hence const
    uint32_t Reload;          // 0x18
};

/*--------------------------------------------------------------------------}
{	DISABLE BASIC IRQ REGISTER BCM2835 ARM Peripheral manual page 117		}
{--------------------------------------------------------------------------*/
typedef union
{
    struct __attribute__((__packed__, aligned(4)))
    {
        unsigned Disable_Timer_IRQ : 1;            // @0 Timer Irq disable
        unsigned Disable_Mailbox_IRQ : 1;          // @1 Mailbox Irq disable
        unsigned Disable_Doorbell0_IRQ : 1;        // @2 Arm Doorbell0 Irq disable
        unsigned Disable_Doorbell1_IRQ : 1;        // @3 Arm Doorbell0 Irq disable
        unsigned Disable_GPU0_halted_IRQ : 1;      // @4 GPU0 halted IRQ disable
        unsigned Disable_GPU1_halted_IRQ : 1;      // @5 GPU1 halted IRQ disable
        unsigned Disable_Illegal_access_type1 : 1; // @6 Illegal access type 1 IRQ disable
        unsigned Disable_Illegal_access_type0 : 1; // @7 Illegal access type 0 IRQ disable
        unsigned reserved : 24;                    // @8-31 reserved
    };
    uint32_t Raw32; // Union to access all 32 bits as a uint32_t
} irq_disable_basic_reg_t;

/*--------------------------------------------------------------------------}
{	   RASPBERRY PI IRQ HARDWARE REGISTERS - BCM2835 Manual Section 7	    }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) IrqControlRegisters
{
    const irq_basic_pending_reg_t IRQBasicPending; // 0x200   ** Read only hence const
    uint32_t IRQPending1;                          // 0x204
    uint32_t IRQPending2;                          // 0x208
    fiq_control_reg_t FIQControl;                  // 0x20C
    uint32_t EnableIRQs1;                          // 0x210
    uint32_t EnableIRQs2;                          // 0x214
    irq_enable_basic_reg_t EnableBasicIRQs;        // 0x218
    uint32_t DisableIRQs1;                         // 0x21C
    uint32_t DisableIRQs2;                         // 0x220
    irq_disable_basic_reg_t DisableBasicIRQs;      // 0x224
};

#include <stdio.h>
#include "fatfs/ff.h"

FILE *ob_fopen(const char *filename, const char *mode)
{
  FRESULT res;
  BYTE flags = 0;
  FIL *fil;
  int i;

  fil = malloc(sizeof(FIL));
  if (!fil)
    return NULL;

  for (i=0; mode[i] != 0; i++) {
    switch (mode[i]) {
      case 'w':
        flags |= FA_WRITE | FA_CREATE_ALWAYS;
        break;
      case 'r':
        flags |= FA_READ;
        break;
      case '+':
        flags |= FA_READ | FA_WRITE;
        break;
    }
  }

  res = f_open(fil, filename, flags);
  if (res != FR_OK) {
    free(fil);
    return NULL;
  }

  return (FILE *) fil;
}

int ob_fclose(FILE *stream)
{
  FRESULT res;
  FIL *fil = (FIL *) stream;
  res = f_close(fil);
  if (res != FR_OK)
    return -1;

  free(fil);
  return 0;
}
size_t ob_fread(void *ptr, size_t size, size_t count, FILE *stream)
{
  FRESULT res;
  FIL *fil = (FIL *) stream;
  UINT bread;
  res = f_read(fil, ptr, size * count, &bread);
  if (res != FR_OK)
    return 0;

  return bread;
}
size_t ob_fwrite(const void *ptr, size_t size, size_t count, FILE *stream)
{
  FRESULT res;
  FIL *fil = (FIL *) stream;
  UINT bwrite;
  res = f_write(fil, ptr, size * count, &bwrite);
  if (res != FR_OK)
    return 0;

  return bwrite;
}
int ob_fflush(FILE *stream)
{
  FRESULT res;
  FIL *fil;
  if (!stream)
    return 0;

  fil = (FIL *) stream;
  res = f_sync(fil);
  if (res != FR_OK)
    return -1;

  return 0;
}
int ob_feof(FILE *stream)
{
  FIL *fil = (FIL *) stream;
  return f_eof(fil);
}
int ob_fseek(FILE *stream, long offset, int whence)
{
  FRESULT res;
  FIL *fil = (FIL *) stream;
  long o;
  switch (whence) {
    case SEEK_SET:
      o = offset;
      break;
    case SEEK_CUR:
      o = offset + f_tell(fil);
      break;
    case SEEK_END:
      o = f_size(fil) + offset;
      if (o < 0)
        o = 0;
      break;
    default:
      return -1;
  }
  res = f_lseek(fil, o);
  if (res != FR_OK)
    return -1;

  return 0;
}

long ob_ftell(FILE *stream)
{
  FIL *fil = (FIL *) stream;
  return f_tell(fil);
}
//[출처] FatFs로 표준 입출력 함수 사용하기(fopen() 등)|작성자 바람

#define IRQ ((volatile __attribute__((aligned(4))) struct IrqControlRegisters *)(uintptr_t)(PERIPHERAL_BASE + 0xB200))
#define ARMTIMER ((volatile __attribute__((aligned(4))) struct ArmTimerRegisters *)(uintptr_t)(PERIPHERAL_BASE + 0xB400))

uintptr_t TimerIrqSetup(uint32_t period_in_us) // Function to call on interrupt
{
    uint64_t temp;
    uint32_t Buffer[5] = {0};
    ARMTIMER->Control.TimerEnable = FALSE; // Make sure clock is stopped, illegal to change anything while running
    mailbox_tag_message(&Buffer[0], 5, Get_Clock_Rate,
                        8, 8, 4, 0);                // Get GPU clock (it varies between 200-450Mhz)
    Buffer[4] /= 250;                               // The prescaler divider is set to 250 (based on GPU=250MHz to give 1Mhz clock)
    temp = period_in_us;                            // Transfer 32bit to 64bit we need the size
    temp *= Buffer[4];                              // Multiply the period by the clock speed
    temp /= 1000000;                                // Now divid by 1Mhz to get prescaler value
    IRQ->EnableBasicIRQs.Enable_Timer_IRQ = TRUE;   // Enable the timer interrupt IRQ
    ARMTIMER->Load = (uint32_t)(temp & 0xFFFFFFFF); // Set the load value to divisor
    ARMTIMER->Control.Counter32Bit = TRUE;          // Counter in 32 bit mode
    ARMTIMER->Control.Prescale = Clkdiv1;           // Clock divider = 1
    ARMTIMER->Control.TimerIrqEnable = TRUE;        // Enable timer irq
    ARMTIMER->Control.TimerEnable = TRUE;           // Now start the clock
    return 0;
}

void __sync_synchronize() {
    
}

#include <utime.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

int
__utimes (const char *file, const struct timeval tvp[2])
{
  struct utimbuf buf, *times;
  if (tvp)
    {
      times = &buf;
      buf.actime = tvp[0].tv_sec + tvp[0].tv_usec / 1000000;
      buf.modtime = tvp[1].tv_sec + tvp[1].tv_usec / 1000000;
    }
  else
    times = NULL;
  return utime (file, times);
}

int
utime (const char *file, const struct utimbuf *times)
{
  struct timeval timevals[2];
  struct timeval *tvp;
  if (times != NULL)
    {
      timevals[0].tv_sec = (time_t) times->actime;
      timevals[0].tv_usec = 0L;
      timevals[1].tv_sec = (time_t) times->modtime;
      timevals[1].tv_usec = 0L;
      tvp = timevals;
    }
  else
    tvp = NULL;
  return __utimes (file, tvp);
}
weak_alias (__utimes, utimes);


extern void *__dso_handle;
extern void *_fini;