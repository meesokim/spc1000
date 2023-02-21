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

.global _start

.global dmb
.global dsb
.global flush_cache
.global clean_cache
.global restore_context

_start:
    /* kernel.img is loaded at 0x8000
     *
     * 0x2c00 - 0x3c00  User/system stack
     * 0x2800 - 0x2c00  IRQ stack
     * 0x2400 - 0x2800  Abort stack
     * 0x2000 - 0x2400  Supervisor (SWI/SVC) stack
     *
     * All stacks grow down; decrement then store
     *
     * Stack addresses are stored in the stack pointers as
     * 0x80000000+address, as this means the stack pointer doesn't have
     * to change when the MMU is turned on (before the MMU is on, accesses
     * to 0x80000000 go to 0x00000000, and so on). Eventually, the stacks
     * will be given a proper home
     */

    /* SVC stack (for SWIs) at 0x2000 */
    /* The processor appears to start in this mode, but change to it
     * anyway
     */
    cps     #0x13       /* Change to supervisor (SVC) mode */
    add     sp, r4, #0x2400

    /* ABORT stack at 0x2400 */
    cps     #0x17       /* Change to Abort mode */
    add     sp, r4, #0x2800

    /* IRQ stack at 0x2800 */
    cps     #0x12       /* Change to IRQ mode */
    ldr     sp, =__irq_stack_top__

    /* FIQ stack at 0x2c00 */
    cps     #0x11       /* Change to FIQ mode */
    ldr     sp, =__fiq_stack_top__

    /* System stack at 0x3000 */
    cps     #0x1f       /* Change to system mode */
    ldr     sp, =__c_stack_top__

	ldr r1, =RPi_BusAlias
	mov	r0, #0x40000000
	str	r0, [r1]

    /* Stay in system mode from now on */

    /* Zero bss section */
    ldr     r0, =__bss_start__
    ldr     r1, =__bss_end__
    mov     r2, #0
bss_zero_loop:
    cmp     r0,r1
    it      lt
    strlt   r2,[r0], #4
    blt     bss_zero_loop

    /* Enable the FPU */
    mrc     p15, 0, r0, c1, c0, 2
    orr     r0, r0, #0x300000            /* single precision */
    orr     r0, r0, #0xC00000            /* double precision */
    mcr     p15, 0, r0, c1, c0, 2
    mov     r0, #0x40000000
    fmxr    fpexc, r0

    /* Turn on unaligned memory access */
    mrc     p15, #0, r4, c1, c0, #0
    orr     r4, #0x400000   /* 1<22 */
    mcr     p15, #0, r4, c1, c0, #0

    /* Enable MMU */
    ldr     r1, =ttbr0              // addr(TTBR0)

    ldr     r2, =0x0000040E         
    mov     r3, #0                  // from 0x00000000
    mov     r4, #0x200              //   to 0x1FFFFFFF
    bl      set_pgtbl_entry

    ldr     r2, =0x00002416
    mov     r3, #0x200              // from 0x20000000 (incl. peripherals)
    mov     r4, #0x1000             //   to 0xFFFFFFFF
    bl      set_pgtbl_entry

    ldr     r2, =0x0000040E
    mov     r3, #0x480              // framebuffer at 0x48000000
    mov     r4, #0x490              // make 16 Mbyte cacheable
    bl      set_pgtbl_entry

    mov     r3, #3
    mcr     p15, #0, r3, c3, c0, #0 // set domain 0 to master

    mcr     p15, #0, r1, c2, c0, #0 // set TTBR0 (addr of ttbr0)  (ptblwlk inner non cacheable,
                                    // outer non-cacheable, not shareable memory)
    /* Start L1 Cache */
    mov     r3, #0
    mcr     p15, #0, r3, c7, c7, #0 /* Invalidate data cache and flush prefetch buffer */
    mcr     p15, #0, r3, c8, c7, #0 /* Invalidate TLB */
    mrc     p15, #0, r2, c1, c0, #0 /* Read Control Register Configuration Data */
    orr     r2, #0x00800000
    orr     r2, #0x00001000         /* Instruction */
    orr     r2, #0x00000800         /* Branch Prediction */
    orr     r2, #0x00000004         /* Data */
    orr     r2, #0x00000001
    mcr     p15, #0, r2, c1, c0, #0 /* Write Control Register Configuration Data */

;@"========================================================================="
@#                              Enable L1 cache
;@"========================================================================="
.equ SCTLR_ENABLE_DATA_CACHE,			0x4
.equ SCTLR_ENABLE_BRANCH_PREDICTION,	0x800
.equ SCTLR_ENABLE_INSTRUCTION_CACHE,	0x1000
    mrc p15,0,r0,c1,c0,0					;@ R0 = System Control Register

    /* Enable caches and branch prediction */
    orr r0, #SCTLR_ENABLE_BRANCH_PREDICTION
    orr r0, #SCTLR_ENABLE_DATA_CACHE
    orr r0, #SCTLR_ENABLE_INSTRUCTION_CACHE

    mcr p15,0,r0,c1,c0,0					;@ System Control Register = R0    

    /* Enable interrupts */
    ldr     r4, =interrupt_vectors
    mcr     p15, #0, r4, c12, c0, #0
    /* Call constructors of all global objects */
    ldr     r0, =__init_array_start
    ldr     r1, =__init_array_end
globals_init_loop:
    cmp     r0, r1
    it      lt
    ldrlt   r2, [r0], #4
    blxlt   r2
    blt     globals_init_loop
    /* Jump to main */
    bl      main

    /* Hang if main function returns */
hang:
    b       hang

set_pgtbl_entry:
    lsl     r0, r3, #20             // = r3 * 0x100000 (1M)
    orr     r0, r2
    str     r0, [r1, r3, lsl #2]
    add     r3, #1
    cmp     r3, r4
    bne     set_pgtbl_entry
    mov     pc, lr

/*
 * Data memory barrier
 * No memory access after the DMB can run until all memory accesses before it
 * have completed
 */
 
dmb:
    mov     r0, #0
    mcr     p15, #0, r0, c7, c10, #5
    mov     pc, lr

/*
 * Data synchronisation barrier
 * No instruction after the DSB can run until all instructions before it have
 * completed
 */
dsb:
    mov     r0, #0
    mcr     p15, #0, r0, c7, c10, #4
    mov     pc, lr

/*
 * Clean and invalidate entire cache
 * Flush pending writes to main memory
 * Remove all data in data cache
 */
flush_cache:
    mov     r0, #0
    mcr     p15, #0, r0, c7, c14, #1
    mov     pc, lr

clean_cache:
    mov     r0, #0
    MCR     p15, 0, r0, c7, c5, 0
    mov     pc, lr
/*
 * Interrupt vectors table
 */
    .align  5
interrupt_vectors:
    b       bad_exception /* RESET */
    b       bad_exception /* UNDEF */
    b       interrupt_swi
    b       interrupt_prefetch_abort
    b       interrupt_data_abort
    b       bad_exception /* Unused vector */
    b       interrupt_irq
    b       interrupt_fiq /* FIQ */

    .section .data

    .align 14
ttbr0:
    .space  4 << 12                        // 4 bytes * 4096 entries

/* "PROVIDE C FUNCTION: uint32_t GPUaddrToARMaddr (uint32_t BUSaddress);" */
.section .text.GPUaddrToARMaddr, "ax", %progbits
.balign	4
.globl GPUaddrToARMaddr;		
.type GPUaddrToARMaddr, %function
GPUaddrToARMaddr:
	ldr r1, =RPi_ARM_TO_GPU_Alias						;@ Fetch address of bus alias value
    ldr r1,[r1]											;@ Fetch bus alias	
	bic r0, r0, r1										;@ Create arm address
	bx   lr												;@ Return
.balign	4
.ltorg													;@ Tell assembler ltorg data for this code can go here
.size	GPUaddrToARMaddr, .-GPUaddrToARMaddr

;@"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
@#     	          DATA FOR SMARTSTART32 EXPOSED TO INTERFACE 
;@"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
.section ".data.smartstart32", "aw"
.balign 4

.globl RPi_IO_Base_Addr;								;@ Make sure Pi_IO_Base_Addr label is global
RPi_IO_Base_Addr : .4byte 0;							;@ Peripheral Base addr is 4 byte variable in 32bit mode

.globl RPi_ARM_TO_GPU_Alias;							;@ Make sure RPi_ARM_TO_GPU_Alias label is global
RPi_ARM_TO_GPU_Alias: .4byte 0;							;@ ARM to GPU alias is 4 byte variable in 32bit mode

.globl RPi_BootAddr;									;@ Make sure RPi_BootAddr label is global
RPi_BootAddr : .4byte 0;								;@ CPU boot address is 4 byte variable in 32bit mode

.globl RPi_CoresReady;									;@ Make sure RPi_CoresReady label is global
RPi_CoresReady : .4byte 0;								;@ CPU cores ready for use is 4 byte variable in 32bit mode

.globl RPi_CPUBootMode;									;@ Make sure RPi_CPUBootMode label is global
RPi_CPUBootMode : .4byte 0;								;@ CPU Boot Mode is 4 byte variable in 32bit mode

.globl RPi_CpuId;										;@ Make sure RPi_CpuId label is global
RPi_CpuId : .4byte 0;									;@ CPU Id is 4 byte variable in 32bit mode

.globl RPi_CompileMode;									;@ Make sure RPi_CompileMode label is global
RPi_CompileMode : .4byte 0;								;@ Compile mode is 4 byte variable in 32bit mode

.globl RPi_CPUCurrentMode;								;@ Make sure RPi_CPUCurrentMode label is global
RPi_CPUCurrentMode : .4byte 0;							;@ CPU current Mode is 4 byte variable in 32bit mode

.globl RPi_SmartStartVer;								;@ Make sure RPi_SmartStartVer label is global
RPi_SmartStartVer : .4byte 0x00020102;					;@ SmartStart version is 4 byte variable in 32bit mode

.global ulCriticalNesting;
ulCriticalNesting : .8byte 0x9999;

.global pxCurrentTCB;
pxCurrentTCB : .4byte 0;

;@"*************************************************************************"
;@"          INTERNAL DATA FOR SMARTSTART NOT EXPOSED TO INTERFACE			"
;@"*************************************************************************"
.section ".data.smartstart", "aw"
.balign 4

RPi_BusAlias	: .4byte 0;				// Address offset between VC4 physical address and ARM address needed for all DMA


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++}
{		VC4 GPU ADDRESS HELPER ROUTINES PROVIDE BY RPi-SmartStart API	    }
{++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* "PROVIDE C FUNCTION: uint32_t ARMaddrToGPUaddr (void* ARMaddress);" */
.section .text.ARMaddrToGPUaddr, "ax", %progbits
.balign	4
.globl ARMaddrToGPUaddr;		
.type ARMaddrToGPUaddr, %function
.syntax unified
.arm
;@"================================================================"
;@ ARMaddrToGPUaddr -- Composite Pi1, Pi2 & Pi3 code
;@ C Function: uint32_t ARMaddrToGPUaddr (void* ARMaddress);
;@ Entry: R0 will have ARMAddress value
;@"================================================================"
ARMaddrToGPUaddr:
	ldr r1, =RPi_BusAlias
    ldr r1,[r1]								;@ Fetch bus alias	
	orr r0, r0, r1							;@ Create bus address
	bx   lr									;@ Return
.balign	4
.ltorg										;@ Tell assembler ltorg data for this code can go here
.size	ARMaddrToGPUaddr, .-ARMaddrToGPUaddr



