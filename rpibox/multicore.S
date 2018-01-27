	.equ	SCTLR_ENABLE_DATA_CACHE,        0x4
	.equ	SCTLR_ENABLE_BRANCH_PREDICTION, 0x800
	.equ	SCTLR_ENABLE_INSTRUCTION_CACHE, 0x1000

;@"================================================================"
;@ Same spin as the STUB does but tolerant on registers R0-R3 for C.
;@"================================================================"
SecondarySpin:
	mrc     p15, 0, r0, c0, c0, 5
	ubfx    r0, r0, #0, #2					// Read core id and make a bit mask in R0 
	ldr r5, =mbox		
	ldr r5, [r5]		@ mbox
	mov	r3, #0			@ magic
	add	r5, #(0x400000CC-0x4000008C)	@ mbox
1:
	ldr	r4, [r5, r0, lsl #4]
	cmp	r4, r3
	beq	1b
@ clear mailbox
	str	r4, [r5, r0, lsl #4]
	mov	r0, #0
	ldr r1, =machid		
	ldr r1, [r1]		@ BCM2708 machine id
	ldr r2, = atags		
	ldr r2, [r2]		@ ATAGS
	ldr lr, =SecondarySpin
	bx	r4
	b SecondarySpin
mbox: 	.4byte 0x4000008C
machid:	.4byte 3138
atags:  .4byte 0x100

.globl setStackPtr; 
setStackPtr : .4byte 0;
.globl setIrqStackPtr; 
setIrqStackPtr : .4byte 0;
.globl ExtraCoreSetup;
ExtraCoreSetup:
    ldr r1, =setStackPtr				;@ Address of User stack pointer value
	ldr sp, [r1]						;@ Set the stack pointer for that mode
;@"================================================================"
;@ If the cpu is in HYP_MODE(EL2) we will bring it SVC_MODE (EL1).
;@"================================================================"
    mrs r0,cpsr							;@ Fetch the cpsr register which includes CPU mode bits 
 	and r1, r0, #0x1F					;@ Mask off the CPU mode bits to register r1                            
 	cmp r1, #0x1A						;@ check we are in HYP_MODE AKA register reads 1A											
	bne .NotInHypMode1					;@ Branch if not equal meaning was not in HYP_MODE  
	bic r0,r0,#0x1F						;@ Clear the CPU mode bits in register r0							
   	orr r0,r0,#0x13						;@ Logical OR SVC_MODE bits onto register
    msr spsr_cxsf,r0					;@ Hold value in spsr_cxsf
    add lr,pc,#4						;@ Calculate address of .NotInHypMode label
	/* I borrowed this trick from Ultibo because ARM6 code running on an ARM7/8 needs this opcode  */
	/* The ARM6 compiler does not know these instructions so it is a way to get needed opcode here */
    /* So our ARM6 code can drop an arm7 or arm8 out of HYP mode and run on an ARM7/8.             */
 	/* Native ARM7/8 compilers already understand the OPCODE but do not mind it this way either	   */        
	.long 0xE12EF30E					;@ "msr ELR_hyp, lr" Set the address to ELR_hyp
	.long 0xE160006E					;@ "eret" Elevated return which will exit at .NotInHypMode in SVC_MODE
.NotInHypMode1:
;@"================================================================"
;@ Now setup stack pointers for the different CPU operation modes.
;@"================================================================"
	msr CPSR_c, #0xD2					;@ Switch to IRQ_MODE
    ldr r1, =setIrqStackPtr
	ldr sp, [r1]						;@ Set the stack pointer for that mode
	msr CPSR_c, #0xD3					;@ Switch back to SRV_MODE
    ldr r1, =setStackPtr
	ldr sp, [r1]						;@ Set the stack pointer for that mode
@"================================================================"
;@ PI NSACR regsister setup for access to floating point unit
;@ Cortex A-7 => Section 4.3.34. Non-Secure Access Control Register
;@ Cortex A-53 => Section 4.5.32. Non-Secure Access Control Register
;@"================================================================"
	mrc p15, 0, r0, c1, c1, 2			;@ Read NSACR into R0
	cmp r0, #0x00000C00					;@ Access turned on or in AARCH32 mode and can not touch register or EL3 fault
	beq .free_to_enable_fpu1
	orr r0, r0, #0x3<<10				;@ Set access to both secure and non secure modes
	mcr p15, 0, r0, c1, c1, 2			;@ Write NSACR
;@"================================================================"
;@ Bring fpu online
;@"================================================================"
.free_to_enable_fpu1:
	mrc p15, 0, r0, c1, c0, #2			;@ R0 = Access Control Register
	orr r0, #(0x300000 + 0xC00000)		;@ Enable Single & Double Precision
	mcr p15,0,r0,c1,c0, #2				;@ Access Control Register = R0
	mov r0, #0x40000000					;@ R0 = Enable VFP
	vmsr fpexc, r0						;@ FPEXC = R0
;@"================================================================"
;@ Enable L1 cache
;@"================================================================"
    mrc p15,0,r0,c1,c0,0				;@ R0 = System Control Register

    /* Enable caches and branch prediction */
    orr r0, #SCTLR_ENABLE_BRANCH_PREDICTION
    orr r0, #SCTLR_ENABLE_DATA_CACHE
    orr r0, #SCTLR_ENABLE_INSTRUCTION_CACHE

    mcr p15,0,r0,c1,c0,0				;@ System Control Register = R0
;@"================================================================"
;@ Acknowledge stacks set by clearing values
;@"================================================================"
	mov r0, #0
	ldr r1, =setIrqStackPtr
	str r0, [r1]
    ldr r1, =setStackPtr
	str r0, [r1]
	ldr lr, =SecondarySpin
	bx lr								;@ Always return to secondary spin	
	bl SecondarySpin					;@ Just safety .. to be sure to be sure
/*	
.globl setup
setup:
    mrs x0,mpidr_el1
    mov x1,#0xFF000000
    bic x0,x0,x1
    cbz x0,zero
    sub x1,x0,#1
    cbz x1,one
    sub x1,x0,#2
    cbz x1,two
    sub x1,x0,#3
    cbz x1,three

    mrs x0,mpidr_el1
    mov x1,#0x40000
    str w0,[x1]
    b hang

zero:
    mov sp,#0x8000
    bl notmain
hang: b hang

one:
    mrs x0,mpidr_el1
    mov x1,#0x10000
    str w0,[x1]
    b hang

two:
    mrs x0,mpidr_el1
    mov x1,#0x20000
    str w0,[x1]
    b hang

three:
    mrs x0,mpidr_el1
    mov x1,#0x30000
    str w0,[x1]
    b hang	
*/