#include "z80-cpu.h"
#include "z80-mon.h"
#include "execute_token"
#include "execute.h"
#include "decode-table.h"
#include "regs_token"
#include "memory.h"
#ifdef  FURTHER_HARDWARE
#include "hardware/daisy_chain.h"
#endif

_ushort ddfdcb_reg; /* indicates third argument in case FDCB/DDCB instruction */


/* converts hl instruction to ix version */
static int
todd(_uchar *t1,_uchar *a1,_uchar *t2,_uchar *a2)
{
  if (*t1==A_REG && *a1==R_HL) *a1=R_IX;
  if (*t2==A_REG && *a2==R_HL) *a2=R_IX;
  if (*t1==A_PODLE_REG && *a1==R_HL)
     *t1=A_PODLE_IX_PLUS,  *a1=1;
  else
  if (*t2==A_PODLE_REG && *a2==R_HL)
     *t2=A_PODLE_IX_PLUS,  *a2=1;
  else
  {
    if (*t2==A_REG && *a2==R_L) { IXl=IX&0xff; IXh=IX>>8; *a2=R_IXl; }
    if (*t2==A_REG && *a2==R_H) { IXl=IX&0xff; IXh=IX>>8; *a2=R_IXh; }
    if (*t1==A_REG && *a1==R_L) { IXl=IX&0xff; IXh=IX>>8; *a1=R_IXl; return 1; }
    if (*t1==A_REG && *a1==R_H) { IXl=IX&0xff; IXh=IX>>8; *a1=R_IXh; return 1; }
  }
  return 0;
}


/* converts hl instruction to iy version */
static int
tofd(_uchar *t1,_uchar *a1,_uchar *t2,_uchar *a2)
{
  if (*t1==A_REG && *a1==R_HL) *a1=R_IY;
  if (*t2==A_REG && *a2==R_HL) *a2=R_IY;
  if (*t1==A_PODLE_REG && *a1==R_HL)
     *t1=A_PODLE_IY_PLUS,  *a1=1;
  else
  if (*t2==A_PODLE_REG && *a2==R_HL)
     *t2=A_PODLE_IY_PLUS,  *a2=1;
  else
  {
    if (*t2==A_REG && *a2==R_L) { IYl=IY&0xff; IYh=IY>>8; *a2=R_IYl; }
    if (*t2==A_REG && *a2==R_H) { IYl=IY&0xff; IYh=IY>>8; *a2=R_IYh; }
    if (*t1==A_REG && *a1==R_L) { IYl=IY&0xff; IYh=IY>>8; *a1=R_IYl; return 1; }
    if (*t1==A_REG && *a1==R_H) { IYl=IY&0xff; IYh=IY>>8; *a1=R_IYh; return 1; }
  }
  return 0;
}


void refresh_cycle(void)
{ if (cpu_is_in_disassemble) return;
  R++; if (!(R&127)) R ^= 128;
  ADDRESS= I<<8 | R;
  set_cpu_pin(mreq,1);
  set_cpu_pin(rfsh,1);
  wait_tics(1);
  set_cpu_pin(rfsh,0);
  set_cpu_pin(mreq,0);
}


/* reads, decodes and call execution of instruction */
/* exec: 0=disassemble instruction and print it                    *
 *       1=execute instruction                                     *
 *       2=only adjust PC according to instruction length          *
 * new_pc: pointer to a value to set PC after calling instruction  *
           (PC untouched) or null if PC should increase            */
void
decode(_ushort *new_pc,int exec)
{
 static struct instruction_type current_instr;
 bit iy=0, ddfd_prefix=0, join=0;
 _uchar disp=0, d;
 _ushort a1,a2;
 struct instruction_type *p;
 
 bit old_cpu_mode= cpu_is_in_disassemble;
 cpu_is_in_disassemble= (exec != 1);
 ddfdcb_reg=0;
 d= read_opcode(PC,1);
 switch (d)
 {
  case 0xcb:    /* 0xcb, opcode */
  p=instr_cb;
  refresh_cycle();
  PC++;
  d= read_opcode(PC,1);
  goto normal;

  case 0xfd:
  case 0xdd:
  do {
    iy= d==0xfd;
    if (ddfd_prefix) f_nop(0,0,0,0);
    ddfd_prefix=1;
    refresh_cycle();
    PC++;
    d= read_opcode(PC,1);
  } while (d==0xdd || d==0xfd);
  if (d==0xed)
  { ddfd_prefix=iy=0;
    goto ed_from_dd_fd;
  }
  if (d!=0xcb)
  {
    p=instr_normal;
    goto normal;   /* 0xdd (0xfd), opcode */
  }
  p=instr_cb;
  refresh_cycle();
  PC++;
  disp= read_opcode(PC,0);
  PC++;  /* no refresh generated for displacement fetch */
  d= read_opcode(PC,0);
  p+= (d&0xf8)|6;
  ddfdcb_reg= (p->func!=f_bit?2+d&7:0);  /* encoding for reg_str in regs.c */
  PC++;  /* no refresh generated for secondary opcode fetch */
  
  current_instr.func=p->func;
  current_instr.arg1=p->arg1;
  current_instr.type1=p->type1;
  current_instr.arg2=p->arg2;
  current_instr.type2=p->type2;
  if (current_instr.type1==A_PODLE_REG)
  {  current_instr.type1=(iy?A_PODLE_IY_PLUS:A_PODLE_IX_PLUS);
     current_instr.arg1=disp;
  }
  if (current_instr.type2==A_PODLE_REG)
  {  current_instr.type2=(iy?A_PODLE_IY_PLUS:A_PODLE_IX_PLUS);
     current_instr.arg2=disp;
  }
  break;

  case 0xed:  /* 0xed, opcode    table for IX and IY instructions is the same */
  ed_from_dd_fd:
  p=instr_ed;
  refresh_cycle();
  PC++;
  d= read_opcode(PC,1);
  goto normal;

  default:  /* opcode */
  p=instr_normal;

  normal:
  p+=d;
  refresh_cycle();
  PC++;
  current_instr.func=p->func;
  current_instr.arg1=p->arg1;
  current_instr.type1=p->type1;
  current_instr.arg2=p->arg2;
  current_instr.type2=p->type2;
  if (p->func == f_djnz && !cpu_is_in_disassemble)
     wait_tics(TICS_LATE_ALU);
  if (ddfd_prefix && p->func != f_exx && (p->func != f_ex || p->type1 != A_REG))
  { if (iy)
    join= tofd(&current_instr.type1,
               &current_instr.arg1,
               &current_instr.type2,
               &current_instr.arg2);
    else
    join= todd(&current_instr.type1,
               &current_instr.arg1,
               &current_instr.type2,
               &current_instr.arg2);
    if (current_instr.type1==A_PODLE_IY_PLUS||current_instr.type1==A_PODLE_IX_PLUS)
    {  current_instr.arg1 = read_opcode(PC,0); PC++; }
    if (current_instr.type2==A_PODLE_IY_PLUS||current_instr.type2==A_PODLE_IX_PLUS)
    {  current_instr.arg2 = read_opcode(PC,0); PC++; }
  }
  break;
 }
 a1= current_instr.arg1; 
 a2= current_instr.arg2; 
 if (current_instr.type1==A_NUM||current_instr.type1==A_PODLE_NUM)
 {
  a1=0;
  if (current_instr.arg1)
  {  a1= read_opcode(PC,0); PC++; }
  if (current_instr.arg1==2)
  {  a1|= read_opcode(PC,0)<<8; PC++; }
 }
 if (current_instr.type2==A_NUM||current_instr.type2==A_PODLE_NUM)
 {
  a2=0;
  if (current_instr.arg2)
  {  a2= read_opcode(PC,0); PC++; }
  if (current_instr.arg2==2)
  {  a2|= read_opcode(PC,0)<<8; PC++; }
 }
 
 if (new_pc)
    PC= *new_pc; /* instruction from user by monitor */
 if (exec!=2)
 {  if (exec==1)
    {  set_cpu_pin(halt,0);
       IFF0=0;
       current_instr.func(a1,current_instr.type1,a2,current_instr.type2);
       if (join)
       {  if (iy)  IY= IYh<<8|IYl;
          else     IX= IXh<<8|IXl;
       }
    }
    else
       print(current_instr.func(a1,current_instr.type1,a2,current_instr.type2));
 }
 if (cpu_is_in_disassemble == (exec!=1))
    cpu_is_in_disassemble=old_cpu_mode;
 return;
}


void
inter_handler(void)
{
   static _uchar tmp_memory[16];
   _ushort new_PC;
   bit old_cpu_mode= cpu_is_in_disassemble;
   cpu_is_in_disassemble= 0;
   if (cpu_pin[halt])  PC+=1;
   IFF2=0;
   IFF1=0;
#ifdef  FURTHER_HARDWARE
   acknowledge_interrupt();
#else
   set_cpu_pin(inter,0);
#endif
/** busrequest was not early uneough */
   set_cpu_pin(iorq,1);
   set_cpu_pin(m1,1);        /* iorq & m1 pin set to acknowledge interrupt */
   wait_tics(2*TICS_WAIT);   /* acknowledge interrupt cycles */
   switch (IM)
   {
      case 0: /* get instruction from data_bus and execute */
/***********************************
a RST n on the data bus, it takes 13 cycles to get to 'n':

    * M1: 5+2 T states: acknowledge interrupt and decrement SP
    * M2: 3 T states: write high byte and decrement SP
    * M3: 3 T states: write low byte and jump to 'n' 

With a CALL nnnn on the data bus, it takes 19 cycles:

    * M1: 4+2 T states: acknowledge interrupt
    * M2: 3 T states: read low byte of 'nnnn' from data bus
    * M3: 4 T states: read high byte of 'nnnn' and decrement SP
    * M4: 3 T states: write high byte of PC to the stack and decrement SP
    * M5: 3 T states: write low byte of PC and jump to 'nnnn'. 
*****************************/
              io_address=tmp_memory;
              PC=0;
#ifdef  FURTHER_HARDWARE
              DATA=read_request_to_peripheral();
#endif
              tmp_memory[0]= DATA; /* all pins high on bus ==> RST 38H */
              decode(0,1);
              set_cpu_pin(iorq,0);
              io_address=0;
              break;
      case 1:
/***********************************
The processor takes 13 T states to reach #0038:
    * M1: 5+2 T states: acknowledge the interrupt and decrement SP
    * M2: 3 T states: write the high byte of PC onto the stack and decrement SP
    * M3: 3 T states: write the low byte onto the stack and to set PC to #0038.
************************************/
              io_address=tmp_memory;
              PC=0;
              tmp_memory[0]= 0xff; /* RST 38 H */
              decode(0,1);
              set_cpu_pin(iorq,0);
              io_address=0;
              break;
      case 2:
/************************************
    * M1: 5+2 T states: acknowledge interrupt and decrement SP
    * M2: 3 T states: write high byte and decrement SP
    * M3: 3 T states: write low byte
    * M4: 3 T states: read low byte from the interrupt vector
    * M5: 3 T states: read high byte and jump to interrupt routine 
*************************************/
              set_cpu_pin(m1,1);
              wait_tics(TICS_MEMO);  /* simulated opcode CALL fetch */
              set_cpu_pin(m1,0);
              refresh_cycle();
#ifdef  FURTHER_HARDWARE
              DATA=read_request_to_peripheral();
#endif
              if (cpu_pin[busrq]) acknowledge_bus_request();
              wait_tics(TICS_MEMO);  /* simulated operand fetch */
              new_PC= (_ushort)(I<<8|DATA);   /* DATA&1  must be zero! */
              if (cpu_pin[busrq]) acknowledge_bus_request();
              wait_tics(TICS_MEMO);  /* simulated operand fetch */
              f_call(new_PC,A_NUM,0,0);
              set_cpu_pin(iorq,0);
              break;
   }
   cpu_is_in_disassemble= old_cpu_mode;
}


void
nmi_handler(void)
{
/********************************
 * M1: 5 T states: opcode read (RST 66) and decrement SP
 * M2: 3 T states: write the high byte of PC to the stack and decrement SP
 * M3: 3 T states: write the low byte of PC and jump to #0066.
********************************/
   bit old_cpu_mode= cpu_is_in_disassemble;
   cpu_is_in_disassemble=0;
   IFF1=0;  /* IFF2 not effected! */
   if (cpu_pin[halt])  PC+=1;
   set_cpu_pin(m1,1);
   wait_tics(TICS_MEMO);  /* simulated opcode fetch */
   set_cpu_pin(m1,0);
   refresh_cycle();
   f_rst((_ushort)0x0066,A_EMPTY,0,0);
   IFF3=0;
   cpu_is_in_disassemble= old_cpu_mode;
}


void
reset_cpu(void)
{
 cpu_is_in_disassemble=0;
 PC=0;
 I=0;R=0;
 IFF0=IFF1=IFF2=IFF3=0;IM=0;
 SP=0xffff;
 A=F=0xff;
 wait_tics(TICS_MEMO);
 set_tics(0);
 set_cpu_pin(halt,0);
 set_cpu_pin(rd,0);
 set_cpu_pin(wr,0);
 set_cpu_pin(m1,0);
 set_cpu_pin(iorq,0);
 set_cpu_pin(mreq,0);
 set_cpu_pin(rfsh,0);
 set_cpu_pin(reset,0);
 cpu_is_in_disassemble=1;
}
