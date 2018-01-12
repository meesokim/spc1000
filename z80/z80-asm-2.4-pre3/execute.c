/* EXECUTE OR DISASSEMBLE INSTRUCTION */
/* Note: I suppose all functions get correct arguments */
/* we compute the additional ticks of the execution unit
 * not covered by memory read/write or refresh cycles */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "z80-cpu.h"
#include "execute_token"
#include "decode.h"
#include "regs.h"
#include "instr.h"
#include "asm.h"
#include "memory.h"
#include "ports.h"
#include "hash.h"

int MODE=0;

#define  VPRE4  "V%04x"
#define  KPRE4  "K%04x"
#define  DECI  "%u"
#define  SDEC  "%+d"
#define  HEXA  "%x"
#define  HEXP  "0x%x"
#define  HEX4  "%04x"
#define  HEX2  "%02x"

/* returns 1 if flag is set, 0 otherwise; on F_EMPTY returns always 1 */
_uchar
is_flag(_uchar f)
{
 int s=5,xor=0;

 switch(f)
 {
  case F_EMPTY:
  return 1;
  
  case F_NC:
  xor=1;
  case F_C:
  s=0;
  break;
  
  case F_NN:
  xor=1;
  case F_N:
  s=1;
  break;
  
  case F_PO:
  xor=1;
  case F_PE:
  s=2;
  break;
  
  case F_NH:
  xor=1;
  case F_H:
  s=4;
  break;
  
  case F_NZ:
  xor=1;
  case F_Z:
  s=6;
  break;
  
  case F_P:
  xor=1;
  case F_M:
  s=7;
  break;
 }
 return ((F&(1<<s))>>s)^xor;
}


void
set_flag(_uchar f)
{
 int a=0,s=5;

 switch(f)
 {
  case F_C:
  a=1;
  case F_NC:
  s=0;
  break;
  
  case F_N:
  a=1;
  case F_NN:
  s=1;
  break;
  
  case F_PE:
  a=1;
  case F_PO:
  s=2;
  break;
  
  case F_H:
  a=1;
  case F_NH:
  s=4;
  break;
  
  case F_Z:
  a=1;
  case F_NZ:
  s=6;
  break;
  
  case F_M:
  a=1;
  case F_P:
  s=7;
  break;
 }
 F&=255-(1<<s);  /* reset apropriate bit in flag register */
 F|=a<<s;        /* set the bit to value a */
}


/* prints register to str */
static void
flag_str (char *str,_ushort arg)
{
 if (arg >= N_FLAGS+1)
    exit( fprintf(stderr,"flag_str: invalid argument %u\n",arg) );
 sprintf(str,"%s",flag_name[arg]);
 return;
}


/* returns pointer to an 8-bit register number n (n=R_A,R_B,...) */
static _uchar * const
reg_ptr(_ushort n)
{
 if (!n || n-1 >= N_8BIT_REGS)
    exit( fprintf(stderr,"reg_ptr: invalid argument %u\n",n) );
 return reg_adr[n-1];
}


static void
reg_str (char *str,_ushort arg,_uchar type)
{  int i;
   switch (type)
   {
      case A_REG:
         if (arg >= N_REGISTERS+1)
            exit( fprintf(stderr,"reg_str: invalid argument %u\n",arg) );
         sprintf(str,"%s",reg_name[arg]);
         return;

      case A_PODLE_REG:
         if (arg >= N_REGISTERS+1)
            exit( fprintf(stderr,"reg_str: invalid argument %u\n",arg) );
         sprintf(str,"(%s)",reg_name[arg]);
         return;

      case A_PODLE_IX_PLUS:
         i=sprintf(str,"(%s",reg_name[R_IX]);
         goto together;

      case A_PODLE_IY_PLUS:
         i=sprintf(str,"(%s",reg_name[R_IY]);
         together:

         if (MODE&2)
         {
            if ((signed char)arg < 0)
               sprintf(str+i,"-"), arg= -(signed char)arg;
            else
               sprintf(str+i,"+");
            sprintf(str+i+1,(MODE&1?"0x"HEX2")":HEXA")"),(unsigned)arg);
         }
         else
            sprintf(str+i,SDEC")",(int)(signed char)arg);
         return;
   }
}


static unsigned short ADDR;


static int add_k_prefix(unsigned short adr)
{  unsigned char *scr;
   if ((MODE&16)!=16)  return 4;
   if (!(scr=malloc(6)))
      return  2;
   sprintf(scr,KPRE4,(unsigned)adr);
   if (!is_in_table(scr,0,0,0))
      return  add_to_table(scr,adr,0,0) ? 3 : 0;  /* error 3 else 0 added */
   else
      return  free(scr), 1;
}


static int add_prefix(char *txt, unsigned short adr)
{  unsigned char *scr;
   if ((MODE&16)!=16)  return 4;
   if (!is_in_table(txt,0,0,0))
      if (scr=malloc(6))
      {  memcpy(scr,txt,5);
         scr[5]=0;
         return  add_to_table(scr,adr,0,0) ? 3 : 0;  /* error 3 else 0 added */
      }
      else
         return 2;
   else
      return 1;
}


/* sets parity according to byte */
static void
__parity(_uchar byte)
{
 int a;
 _uchar v;
 for (a=0,v=0;a<8;a++,byte>>=1)
    v^=(byte)&1;
 set_flag(v?F_PO:F_PE);
}


/* increase PC */
static void
add_to_pc(short incr)
{  if (cpu_pin[busrq]) acknowledge_bus_request();
   PC += incr;
   wait_tics(TICS_JR);
}


/* calculate address with displacement */
static _ushort
ix_iy_disp(_ushort XY, _ushort a, int parallel)
{
   if (!parallel)
      if (cpu_pin[busrq]) acknowledge_bus_request();
/* displacement calculation could be parallel to 2nd operand fetch */
   wait_tics(TICS_IXY_PLUS-(parallel?TICS_MEMO:0));
   return  XY+(signed char)a;
}


static char * dis(int icode)
{
unsigned  i;
for (i=0;i<N_INSTRUCTIONS;i++)
   if (instruction[i].code == icode)
      return  instruction[i].name;
return  "NOP2";
}

_uchar *
f_nop (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 a1=a1;t1=t1;a2=a2;t2=t2;
 return  cpu_is_in_disassemble ? dis(I_NOP) : 0;
}


_uchar *
f_ld (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char arg1[16];
 static char arg2[16];
 static char txt[32];
 _ushort rval=0;
 
 if(cpu_is_in_disassemble)
 {
  if(t1==A_PODLE_NUM)
  {
   sprintf(arg1,((MODE&8)==8?"("KPRE4")":(MODE&3)==3?"(0x"HEX4")":MODE&1?"("DECI")":"("HEX4")"),a1);
   add_k_prefix(a1);
  }
  else
   reg_str(arg1,a1,t1);

  if(t2==A_PODLE_NUM)
  {sprintf(arg2,((MODE&8)==8?"("KPRE4")":(MODE&3)==3?"(0x"HEX4")":MODE&1?"("DECI")":"("HEX4")"),a2);
   add_k_prefix(a2);
  }
  else if(t2==A_NUM)
  {if ((MODE&12)==12  &&  a1 >= N_8BIT_REGS+1 && a2 >= 256)
   {  sprintf(arg2,VPRE4,a2);
      add_prefix(arg2,a2);
   }
   else
      sprintf(arg2,(MODE&2?HEXP:DECI),a2);
  }
  else
   reg_str(arg2,a2,t2);
 
  sprintf(txt,"%s %s,%s",dis(I_LD),arg1,arg2);
  return txt;
 }
 else
 {
  switch(t2)
  {
   case A_NUM:
   rval=a2;
   break;

   case A_PODLE_NUM:
   rval= read_memo(a2);
   if (a1 != R_A) rval |= read_memo((_ushort)(a2+1))<<8;
   break;

   case A_PODLE_REG:
   switch(a2)
   {
    case R_BC:
    ADDR= B<<8|C;
    break;

    case R_DE:
    ADDR= D<<8|E;
    break;

    case R_HL:
    ADDR= H<<8|L;
    break;
   }
   rval= read_memo(ADDR);
   break;

   case A_REG:
   switch(a2)
   {
    case R_BC:
    rval=C|(B<<8);
    break;
 
    case R_DE:
    rval=E|(D<<8);
    break;
    
    case R_HL:
    rval=H<<8|L;
    wait_tics(t1 == A_REG ? TICS_LD_SP : 0);  /* LD SP, HL */
    break;
 
    case R_IX:
    rval=IX;
    wait_tics(t1 == A_REG ? TICS_LD_SP : 0);
    break;
 
    case R_IY:
    rval=IY;
    wait_tics(t1 == A_REG ? TICS_LD_SP : 0);
    break;
    
    case R_SP:
    rval=SP;
    break;
 
    default: /* 8BIT_REG */
    rval= *reg_ptr(a2);
    if (t1 == A_REG)
    {
       if (a2 == R_R || a2 == R_I)
         set_flag(IFF2?F_PO:F_PE);
       if (a2 == R_R || a2 == R_I || a1 == R_R || a1 == R_I)
         wait_tics(TICS_LD_IR);
    }
    break;
   }
   break;

   case A_PODLE_IX_PLUS:
   rval= read_memo(ix_iy_disp(IX,a2,0));
   break;
 
   case A_PODLE_IY_PLUS:
   rval= read_memo(ix_iy_disp(IY,a2,0));
   break;
  }
  switch(t1)
  {
   case A_PODLE_NUM:
   write_memo(a1,(_uchar)rval);
   if (a2 != R_A)
      write_memo((_ushort)(a1+1),(_uchar)(rval>>8));
   break;

   case A_PODLE_IX_PLUS:
   write_memo(ix_iy_disp(IX,a1,(t2==A_NUM)),(_uchar)rval);
   break;
   
   case A_PODLE_IY_PLUS:
   write_memo(ix_iy_disp(IY,a1,(t2==A_NUM)),(_uchar)rval);
   break;

   case A_PODLE_REG:
   switch(a1)
   {
    case R_BC:
    ADDR= B<<8|C;
    break;

    case R_DE:
    ADDR= D<<8|E;
    break;

    case R_HL:
    ADDR= H<<8|L;
    break;
   }
   write_memo(ADDR,(_uchar)rval);
   break;

   case A_REG:
   switch(a1)
   {
    case R_IX:
    IX=rval;
    break;

    case R_IY:
    IY=rval;
    break;

    case R_SP:
    SP=rval;
    break;

    case R_BC:
    C=rval&255;
    B=rval>>8;
    break;

    case R_DE:
    E=rval&255;
    D=rval>>8;
    break;

    case R_HL:
    L=rval&255;
    H=rval>>8;
    break;

    default:
    *reg_ptr(a1)=(_uchar)rval;
    break;
   }
   break;
  }
 }

 if (t1==A_REG&&t2==A_REG&&a1==R_A&&(a2==R_I||a2==R_R))
 {
  set_flag(F_NH);
  set_flag(F_NN);
  set_flag(F_PE);
  set_flag(A?F_NZ:F_Z);
  set_flag((A&128)?F_M:F_P);
 }
 return 0;
}


_uchar *
f_call (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char txt[16];
 static char t[4];
 _ushort addr=0;
 
 t2=t2;
 if(cpu_is_in_disassemble)
 {int i;
  if (t1==A_FLAG)
  {
   flag_str(t,a1);
   i=sprintf(txt,"%s %s,",dis(I_CALL),t);
   a1=a2;
  }
  else
   i=sprintf(txt,"%s ",dis(I_CALL));
  if (MODE&8)
  {  sprintf(txt+i,KPRE4,a1);
     add_k_prefix(a1);
  }
  else
     sprintf(txt+i,((MODE&3)==3?"0x"HEX4:MODE&1?DECI:HEX4),a1);
  return txt;
 }
 else
 {
  switch (t1)
  {
   case A_FLAG:
   if (!is_flag(a1)) return 0;
   addr=a2;
   break;
   
   default:
   addr=a1;
   break;
  }
  wait_tics(TICS_SP_DEC);
  SP--;
  write_memo(SP,PC>>8);
  SP--;
  write_memo(SP,PC&255);
  PC=addr;
 }
 return 0;
}


_uchar *
f_ret (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char txt[8];
 static char t[4];
 
 a2=a2;t2=t2;t1=t1;
 if(cpu_is_in_disassemble)
 {
  flag_str(t,a1);
  sprintf(txt,"%s %s",dis(I_RET),t);
  return txt;
 }
 else
 {
  wait_tics(a1==F_EMPTY ? 0 : TICS_FLAG);
  if (is_flag(a1)) /* empty flag is always set */
  {
   PC= read_memo(SP);
   SP++;
   PC|= read_memo(SP)<<8;
   SP++;
  }
 }
 return 0;
}


_uchar *
f_inc (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char txt[16];
 static char t[10];
 _uchar *a, d;
 unsigned char z=is_flag(F_Z),s=is_flag(F_M),h=is_flag(F_H),v=0, calc_flags=0;
 
 a2=a2;t2=t2;
 if(cpu_is_in_disassemble)
 {
  reg_str(t,a1,t1);
  sprintf(txt,"%s %s",dis(I_INC),t);
  return txt;
 }
 else
 {
  switch(t1)
  {
   case A_REG:
   switch(a1)
   {
    case R_IX:
    IX++;
    wait_tics(TICS_INC_DEC_16);
    break;
    
    case R_IY:
    IY++;
    wait_tics(TICS_INC_DEC_16);
    break;
    
    case R_SP:
    SP++;
    wait_tics(TICS_INC_DEC_16);
    break;
    
    case R_DE:
    E++; D= E?D:D+1;
    wait_tics(TICS_INC_DEC_16);
    break;
    
    case R_HL:
    L++; H= L?H:H+1;
    wait_tics(TICS_INC_DEC_16);
    break;
    
    case R_BC:
    C++; B= C?B:B+1;
    wait_tics(TICS_INC_DEC_16);
    break;
    
    default:
    a=reg_ptr(a1);
    h=(((*a)&15)==15);
    (*a)++;
    v=(*a==128)?1:0;
    z=!(*a);
    s=((*a)&128);
    calc_flags=1;
    break;
   }
   break;

   case A_PODLE_REG:
   ADDR=H<<8|L;
   goto inc;

   case A_PODLE_IY_PLUS:
   ADDR= ix_iy_disp(IY,a1,0);
   goto inc;
   
   case A_PODLE_IX_PLUS:
   ADDR= ix_iy_disp(IX,a1,0);

   inc:
   d= read_memo(ADDR);
   h=((d&15)==15);
   d++;
   v=(d==0x80);
   wait_tics(TICS_LATE_ALU);
   z=!d;
   s=(d&128);
   write_memo(ADDR,d);
   calc_flags=1;
   break;
  }
  if (calc_flags)
  {
   set_flag(F_NN);
   set_flag(z?F_Z:F_NZ);
   set_flag(s?F_M:F_P);
   set_flag(h?F_H:F_NH);
   set_flag(v?F_PE:F_PO);
  }
 }
 return  0;
}


_uchar *
f_dec (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char txt[16];
 static char t[10];
 _uchar *a, d;
 unsigned char z=is_flag(F_Z),s=is_flag(F_M),h=is_flag(F_H),v=0, calc_flags=0;
 
 a2=a2;t2=t2;
 if(cpu_is_in_disassemble)
 {
  reg_str(t,a1,t1);
  sprintf(txt,"%s %s",dis(I_DEC),t);
  return txt;
 } 
 else
 {
  switch(t1)
  {
   case A_REG:
   switch(a1)
   {
    case R_IX:
    IX--;
    wait_tics(TICS_INC_DEC_16);
    break;
    
    case R_IY:
    IY--;
    wait_tics(TICS_INC_DEC_16);
    break;
    
    case R_SP:
    SP--;
    wait_tics(TICS_INC_DEC_16);
    break;
    
    case R_DE:
    D= E?D:D-1; E--;
    wait_tics(TICS_INC_DEC_16);
    break;
    
    case R_HL:
    H= L?H:H-1; L--;
    wait_tics(TICS_INC_DEC_16);
    break;
    
    case R_BC:
    B= C?B:B-1; C--;
    wait_tics(TICS_INC_DEC_16);
    break;
    
    default:
    a=reg_ptr(a1);
    h=!((*a)&15);
    v=(*a==128)?1:0;
    (*a)--;
    z=!(*a);
    s=((*a)&128);
    calc_flags=1;
    break;
   }
   break;

   case A_PODLE_REG:
   ADDR= H<<8|L;
   goto dec;

   case A_PODLE_IY_PLUS:
   ADDR= ix_iy_disp(IY,a1,0);
   goto dec;

   case A_PODLE_IX_PLUS:
   ADDR= ix_iy_disp(IX,a1,0);

   dec:
   d= read_memo(ADDR);
   h=!(d&15);
   v=(d==0x80);
   d--;
   write_memo(ADDR,d);
   z=!d;
   s=(d&128);
   wait_tics(TICS_LATE_ALU);
   calc_flags=1;
   break;
  }
  if (calc_flags)
  {
   set_flag(F_N);
   set_flag(z?F_Z:F_NZ);
   set_flag(s?F_M:F_P);
   set_flag(h?F_H:F_NH);
   set_flag(v?F_PE:F_PO);
  }
 }
 return 0;
}


static _uchar *
f_rot_a (_ushort a1,_uchar t1,_ushort a2,_uchar t2,
         char *str, void (*func)(void) )
{
 a2=a2;t2=t2;a1=a1;t1=t1;
 if(cpu_is_in_disassemble)
  return str;
 else
 {
  func();
  set_flag(F_NN);
  set_flag(F_NH);
 }
 return 0;
}

static void rra(void)
{
  _uchar b=is_flag(F_C);
  set_flag((A&1)?F_C:F_NC);
  A>>=1;
  A|=b<<7;
}

_uchar *
f_rra (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_rot_a (a1,t1,a2,t2,dis(I_RRA),rra);
}

static void rla(void)
{
  _uchar b=is_flag(F_C);
  set_flag((A&128)?F_C:F_NC);
  A<<=1;
  A|=b;
}

_uchar *
f_rla (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_rot_a (a1,t1,a2,t2,dis(I_RLA),rla);
}

static void rrca(void)
{
  _uchar b=A&1;
  set_flag(b?F_C:F_NC);
  A>>=1;
  A|=b<<7;
}

_uchar *
f_rrca (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_rot_a (a1,t1,a2,t2,dis(I_RRCA),rrca);
}

static void rlca(void)
{
  _uchar b=A&128;
  set_flag(b?F_C:F_NC);
  A<<=1;
  A|=b>>7;
}

_uchar *
f_rlca (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_rot_a (a1,t1,a2,t2,dis(I_RLCA),rlca);
}
 

static _uchar *
f_alu (_ushort a1,_uchar t1,_ushort a2,_uchar t2,
       char *str, _uchar (*func)(_uchar) )
{
 static char txt[16];
 static char t[10], tt[4];
 _uchar a=0;
 
 a2=a2;t2=t2;
 if(cpu_is_in_disassemble)
 {
  if (t2==A_EMPTY)
  {
    if (t1==A_NUM)
     sprintf(t,(MODE&2?HEXP:DECI),a1);
    else
     reg_str(t,a1,t1);
    sprintf(txt,"%s %s",str,t);
  }
  else
  {
    reg_str(tt,a1,t1);
    if (t2==A_NUM)
     sprintf(t,(MODE&2?HEXP:DECI),a2);
    else
     reg_str(t,a2,t2);
    sprintf(txt,"%s %s,%s",str,tt,t);
  }
  return txt;
 } 
 else
 {
  if (t2!=A_EMPTY)
     t1=t2, a1=a2;
  switch(t1)
  {
   case A_REG:
   a= *reg_ptr(a1);
   break;

   case A_NUM:
   a=a1;
   break;
   
   case A_PODLE_REG:
   ADDR= H<<8|L;
   a= read_memo(ADDR);
   break;

   case A_PODLE_IY_PLUS:
   ADDR= ix_iy_disp(IY,a1,0);
   a= read_memo(ADDR);
   break;
   
   case A_PODLE_IX_PLUS:
   ADDR= ix_iy_disp(IX,a1,0);
   a= read_memo(ADDR);
   break;
  }
  a=func(a);
  set_flag(a?F_NZ:F_Z);
  set_flag(a&128?F_M:F_P);
 }
 return 0;
}

static _uchar add(_uchar a)
{
   int res,v,c,h;
   res=(signed char)A+(signed char)a;
   c=A>=256-a;
   v=res>127 || res<-128;
   h=((int)(A&15)+(a&15))>15;
   A=(_uchar)res;
   set_flag(v?F_PE:F_PO);
   set_flag(c?F_C:F_NC);
   set_flag(h?F_H:F_NH);
   set_flag(F_NN);
   return A;
}

static _uchar adc(_uchar a)
{
   int res,v,c,h;
   res=(signed char)A+(signed char)a+(int)is_flag(F_C);
   c=A>=256-a-(unsigned)is_flag(F_C);
   v=res>127 || res<-128;
   h=((int)(A&15)+(a&15)+is_flag(F_C))>15;
   A=(_uchar)res;
   set_flag(v?F_PE:F_PO);
   set_flag(c?F_C:F_NC);
   set_flag(h?F_H:F_NH);
   set_flag(F_NN);
   return A;
}

static _uchar sbc(_uchar a)
{
   int res,v,c,h;
   res=(signed char)A-(signed char)a-(int)is_flag(F_C);
   c=A<a+(unsigned)is_flag(F_C);
   v=res>127 || res<-128;
   h=((int)(A&15)-(a&15)-is_flag(F_C))<0;
   A=(_uchar)res;
   set_flag(v?F_PE:F_PO);
   set_flag(c?F_C:F_NC);
   set_flag(h?F_H:F_NH);
   set_flag(F_N);
   return A;
}

static _uchar sub(_uchar a)
{
   int res,v,c,h;
   res=(signed char)A-(signed char)a;
   c=A<a;
   v=res>127 || res<-128;
   h=((int)(A&15)-(a&15))<0;
   A=(_uchar)res;
   set_flag(v?F_PE:F_PO);
   set_flag(c?F_C:F_NC);
   set_flag(h?F_H:F_NH);
   set_flag(F_N);
   return A;
}

static _uchar cp(_uchar a)
{
   int res,v,c,h;
   res=(signed char)A-(signed char)a;
   c=A<a;
   v=res>127 || res<-128;
   h=((int)(A&15)-(a&15))<0;
   set_flag(v?F_PE:F_PO);
   set_flag(c?F_C:F_NC);
   set_flag(h?F_H:F_NH);
   set_flag(F_N);
   return (_uchar)res;
}

_uchar *
f_sub (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_alu (a1,t1,a2,t2,dis(I_SUB),sub);
}

_uchar *
f_cp (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_alu (a1,t1,a2,t2,dis(I_CP),cp);
}

static _uchar and(_uchar a)
{
  __parity(A&a);
  set_flag(F_NC);
  set_flag(F_NN);
  set_flag(F_NH);
  return  A&=a;
}

_uchar *
f_and (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_alu (a1,t1,a2,t2,dis(I_AND),and);
}

static _uchar xor(_uchar a)
{
  __parity(A^a);
  set_flag(F_NC);
  set_flag(F_NN);
  set_flag(F_NH);
  return  A^=a;
}

_uchar *
f_xor (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_alu (a1,t1,a2,t2,dis(I_XOR),xor);
}

static _uchar or(_uchar a)
{
  __parity(A|a);
  set_flag(F_NC);
  set_flag(F_NN);
  set_flag(F_NH);
  return  A|=a;
}

_uchar *
f_or (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_alu (a1,t1,a2,t2,dis(I_OR),or);
}


static _uchar *
f_alu_16 (_ushort a1,_uchar t1,_ushort a2,_uchar t2,
          char *str, _ushort (*func)(_ushort,_ushort) )
{
 static char txt[16];
 static char t[8],tt[8];
 _ushort lv=0,rv=0;
 
 if(cpu_is_in_disassemble)
 {
  reg_str(t,a1,t1);
  reg_str(tt,a2,t2);
  sprintf(txt,"%s %s,%s",str,t,tt);
  return txt;
 } 
 else
 {
  unsigned char  index;
  if(a1==R_IY)
     index=2;
  else if(a1==R_IX)
     index=1;
  else
     index=0;
  switch(a2)
  {
    case R_BC:
    rv=B<<8|C;
    break;

    case R_DE:
    rv=D<<8|E;
    break;

    case R_HL:
    rv=H<<8|L;
    break;

    case R_IX:
    rv=IX;
    break;

    case R_IY:
    rv=IY;
    break;

    case R_SP:
    rv=SP;
    break;
  }
  if (cpu_pin[busrq]) acknowledge_bus_request();
  wait_tics(TICS_ADD_SUB_16_PRE);
  switch(index)
  {
    case 0:    /* HL */
    lv=H<<8|L;
    lv= func(lv,rv);
    L=lv&255;H=lv>>8;
    break;

    case 1:    /* IX */
    lv=IX;
    lv= func(lv,rv);
    IX=lv;
    break;

    case 2:    /* IY */
    lv=IY;
    lv= func(lv,rv);
    IY=lv;
    break;
  }
  if (cpu_pin[busrq]) acknowledge_bus_request();
  wait_tics(TICS_ADD_SUB_16_POST);
 }
 return 0; 
}

static _ushort add16(_ushort lv,_ushort rv)
{
   int c,h;
   c=((int)lv+rv)>65535;
   h=((int)(lv>>8&15)+(rv>>8&15))>15;
   set_flag(F_NN);
   set_flag(c?F_C:F_NC);
   set_flag(h?F_H:F_NH);
   return  (_ushort)(lv+rv);
}

_uchar *
f_add (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 if (a1 == R_A)  return f_alu (a1,t1,a2,t2,dis(I_ADD),add);
 else  return  f_alu_16 (a1,t1,a2,t2,dis(I_ADD),add16);
}

static _ushort sbc16(_ushort lv,_ushort rv)
{
   int c,h,v, res;

   res=(short)lv-(short)rv-(int)is_flag(F_C);
   h=((int)(lv>>8&15)-(int)(rv>>8&15)-is_flag(F_C))<0;
   c=lv<rv+(unsigned)is_flag(F_C);
   v=res>32767||res<-32768;

   set_flag(res&32768?F_M:F_P);
   set_flag(v?F_PE:F_PO);
   set_flag(c?F_C:F_NC);
   set_flag(h?F_H:F_NH);
   set_flag((_ushort)res?F_NZ:F_Z);
   set_flag(F_N);
   return  (_ushort) res;
}

_uchar *
f_sbc (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 if (a1 == R_A)  return f_alu (a1,t1,a2,t2,dis(I_SBC),sbc);
 else  return  f_alu_16 (a1,t1,a2,t2,dis(I_SBC),sbc16);
}

static _ushort adc16(_ushort lv,_ushort rv)
{
   int c,h,v, res;

   res=(short)lv+(short)rv+(int)is_flag(F_C);
   h=((int)(lv>>8&15)+(int)(rv>>8&15)+is_flag(F_C))>15;
   c=lv>=65536-rv-(unsigned)is_flag(F_C);
   v=res>32767||res<-32768;
   
   set_flag(res&32768?F_M:F_P);
   set_flag(v?F_PE:F_PO);
   set_flag(c?F_C:F_NC);
   set_flag(h?F_H:F_NH);
   set_flag((_ushort)res?F_NZ:F_Z);
   set_flag(F_NN);
   return (_ushort) res;
}

_uchar *
f_adc (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 if (a1 == R_A)  return f_alu (a1,t1,a2,t2,dis(I_ADC),adc);
 else  return  f_alu_16 (a1,t1,a2,t2,dis(I_ADC),adc16);
}


_uchar *
f_djnz (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char txt[16];
 
 t1=t1;t2=t2;a2=a2;
 if(cpu_is_in_disassemble)
 {int i;
  i=sprintf(txt,"%s ",dis(I_DJNZ));
  if (MODE&8)
  {  sprintf(txt+i,KPRE4,(unsigned)(PC+(signed char)a1));
     add_k_prefix(PC+(signed char)a1);
  }
  else if (MODE&4)
     sprintf(txt+i,HEX4,(unsigned)(PC+(signed char)a1));
  else if (MODE&2)
  {
     if ((signed char)a1 < 0)
        sprintf(txt+i,"-"), a1= -(signed char)a1;
     else
        sprintf(txt+i,"+");
     sprintf(txt+i+1,(MODE&1?"0x"HEX2:HEXA),(unsigned)a1);
  }
  else
     sprintf(txt+i,SDEC,(int)(signed char)a1);
  return txt;
 }
 else
 {
  if (cpu_is_in_x_mode)  add_to_pc(2);
  B--;
  if (B) add_to_pc((short)(signed char)a1);
 }
 return 0;
}


_uchar *
f_jp (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char txt[16];
 static char t[4];
 _ushort addr=0;
 
 t2=t2;
 if(cpu_is_in_disassemble)
 {int i;
  switch (t1)
  {
   case A_FLAG:
   flag_str(t,a1);
   i=sprintf(txt,"%s %s,",dis(I_JP),t);
   if (MODE&8)
   {  sprintf(txt+i,KPRE4,a2);
      add_k_prefix(a2);
   }
   else
      sprintf(txt+i,((MODE&3)==3?"0x"HEX4:MODE&1?DECI:HEX4),a2);
   break;
   
   case A_REG:
   case A_PODLE_REG:
   reg_str(t,a1,t1);
/* sprintf(txt,"%s %s",dis(I_JP),t); */
   sprintf(txt,"%s (%s)",dis(I_JP),t);
   break;

   default:
   i=sprintf(txt,"%s ",dis(I_JP));
   if (MODE&8)
   {  sprintf(txt+i,KPRE4,a1);
      add_k_prefix(a1);
   }
   else
      sprintf(txt+i,((MODE&3)==3?"0x"HEX4:MODE&1?DECI:HEX4),a1);
   break;
  }
  return txt;
 }
 else
 {
  switch (t1)
  {
   case A_FLAG:
   if (!is_flag(a1)) return 0;
   addr=a2;
   break;
   
   case A_REG:
   case A_PODLE_REG:
   switch(a1)
   {
    case R_IX:
    addr=IX;
    break;
    
    case R_IY:
    addr=IY;
    break;
    
    case R_HL:
    addr=H<<8|L;
    break;
    
   }
   break;

   default:
   addr=a1;
   break;
  }
  PC=addr;
 }
 return 0;
}


_uchar *
f_jr (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char txt[16];
 static char t[4];
 
 t2=t2;
 if(cpu_is_in_disassemble)
 {int i;
  if (t1==A_FLAG)
  {
   flag_str(t,a1);
   i=sprintf(txt,"%s %s,",dis(I_JR),t);
   a1=a2;
  }
  else
   i=sprintf(txt,"%s ",dis(I_JR));
  if (MODE&8)
  {  sprintf(txt+i,KPRE4,(unsigned)(PC+(signed char)a1));
     add_k_prefix(PC+(signed char)a1);
  }
  else if (MODE&4)
     sprintf(txt+i,HEX4,(unsigned)(PC+(signed char)a1));
  else if (MODE&2)
  {
     if ((signed char)a1 < 0)
        sprintf(txt+i,"-"), a1= -(signed char)a1;
     else
        sprintf(txt+i,"+");
     sprintf(txt+i+1,(MODE&1?"0x"HEX2:HEXA),(unsigned)a1);
  }
  else
     sprintf(txt+i,SDEC,(int)(signed char)a1);
  return txt;
 }
 else
 {
  if (t1!=A_FLAG || is_flag(a1))
  {  if (cpu_is_in_x_mode)  add_to_pc(2);
     add_to_pc( (t1==A_FLAG) ? (signed char)a2 : (signed char)a1 );
  }
 }
 return 0;
}


_uchar *
f_daa (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 _uchar a=0;
 int h;
 
 a1=a1;t1=t1;a2=a2;t2=t2;
 if(cpu_is_in_disassemble)
  return dis(I_DAA);
 else
 {
  if (is_flag(F_NN))
  {
    if (is_flag(F_C) || (A>>4)+((A&15)>9)>9 ) a+=0x60;
    if (is_flag(F_H) || (A&15)>9) a+=0x06;
    set_flag(a>=0x60?F_C:F_NC);
    h=((int)(A&15)+(a&15))>15;
    A+=a;
  }
  else
  {
    if (is_flag(F_C)) a+=0x60;
    if (is_flag(F_H)) a+=0x06;
    h=((int)(A&15)-(a&15))<0;
    A-=a;
  }
  set_flag(h?F_H:F_NH);
  set_flag(A?F_NZ:F_Z);
  set_flag(A&128?F_M:F_P);
  __parity(A);
 }
 return 0;
}


_uchar *
f_cpl (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 a1=a1;t1=t1;a2=a2;t2=t2;

 if(cpu_is_in_disassemble)
  return dis(I_CPL);
 else
 {
  A^=255;
  set_flag(F_N);
  set_flag(F_H);
 }
 return 0;
}


_uchar *
f_scf (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 a1=a1;t1=t1;a2=a2;t2=t2;

 if(cpu_is_in_disassemble)
  return dis(I_SCF);
 else
 {
  set_flag(F_C);
  set_flag(F_NN);
  set_flag(F_NH);
 }
 return 0;
}


_uchar *
f_ccf (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 a1=a1;t1=t1;a2=a2;t2=t2;

 if(cpu_is_in_disassemble)
  return dis(I_CCF);
 else
 {
  set_flag(is_flag(F_C)?F_NC:F_C);
  set_flag(F_NN);
 }
 return 0;
}


_uchar *
f_halt (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 a1=a1;t1=t1;a2=a2;t2=t2;
 if (cpu_is_in_disassemble)
   return dis(I_HALT);
 else
 {
   set_cpu_pin(halt,1);
   if (!cpu_is_in_x_mode) PC-=1;
 }
 return 0;
}

_uchar *
f_pop (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char txt[8];
 static char t[4];
 _uchar l,h;
 
 a2=a2;t2=t2;
 if(cpu_is_in_disassemble)
 {
  reg_str(t,a1,t1);
  sprintf(txt,"%s %s",dis(I_POP),t);
  return txt;
 }
 else
 {
  l= read_memo(SP);
  SP++;
  h= read_memo(SP);
  SP++;
  switch (a1)
  {
   case R_AF:
   A=h;F=l;
   break;
   
   case R_BC:
   B=h;C=l;
   break;
   
   case R_DE:
   D=h;E=l;
   break;
   
   case R_HL:
   H=h;L=l;
   break;
   
   case R_IX:
   IX=(_ushort)h<<8|l;
   break;

   case R_IY:
   IY=(_ushort)h<<8|l;
   break;
  }
 }
 return 0;
}


_uchar *
f_push (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char txt[8];
 static char t[4];
 _uchar l=0,h=0;
 
 a2=a2;t2=t2;
 if(cpu_is_in_disassemble)
 {
  reg_str(t,a1,t1);
  sprintf(txt,"%s %s",dis(I_PUSH),t);
  return txt;
 }
 else
 {
  switch (a1)
  {
   case R_AF:
   h=A;l=F;
   break;
   
   case R_BC:
   h=B;l=C;
   break;
   
   case R_DE:
   h=D;l=E;
   break;
   
   case R_HL:
   h=H;l=L;
   break;
   
   case R_IX:
   h=(IX>>8)&255;l=IX&255;
   break;

   case R_IY:
   h=(IY>>8)&255;l=IY&255;
   break;
  }
  wait_tics(TICS_SP_DEC);
  SP--;
  write_memo(SP,h);
  SP--;
  write_memo(SP,l);
 }
 return 0;
}


_uchar *
f_rst (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char txt[16];
 a2=a2;t2=t2;t1=t1;

 if(cpu_is_in_disassemble)
 {
  sprintf(txt,"%s 0x"HEX2,dis(I_RST),a1);
  return txt;
 }
 else
 {
  wait_tics(TICS_SP_DEC);
  SP--;
  write_memo(SP,PC>>8);
  SP--;
  write_memo(SP,PC&255);
  PC=a1;
 }
 return 0;
}


_uchar *
f_out (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char txt[16];
 static char tf[5];
 static char ts[4];
 
 if (cpu_is_in_disassemble)
 {
  if (t1==A_PODLE_NUM)
   sprintf(tf,((MODE&3)==3?"0x"HEX2:MODE&1?DECI:HEX2),a1);
  else
   sprintf(tf,"c");
  if (t2 == A_CONST)
   sprintf(ts,"0");
  else
   reg_str(ts,a2,t2);
  sprintf(txt,"%s (%s),%s",dis(I_OUT),tf,ts);
  return txt;
 }
 else
 {
  if (t1==A_PODLE_REG)
    out_byte(C,(t2==A_CONST?0:*reg_ptr(a2)));
  else /* t1 == A_PODLE_NUM */
    out_byte(a1,A);
 }
 return 0;
}


_uchar *
f_in (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char txt[16];
 static char tf[4];
 static char ts[5];
 
 if (cpu_is_in_disassemble)
 {
  if (t2==A_PODLE_NUM)
   sprintf(ts,((MODE&3)==3?"0x"HEX2:MODE&1?DECI:HEX2),a2);
  else
   sprintf(ts,"c");
  if (t1==A_CONST)
   sprintf(tf,"0");
  else
   reg_str(tf,a1,t1);
  sprintf(txt,"%s %s,(%s)",dis(I_IN),tf,ts);
  return txt;
 }
 else
 {
  if (t2==A_PODLE_REG)
  { _uchar dummy;
    _uchar *r= (t1==A_CONST?&dummy:reg_ptr(a1));
    in_byte(C,r);
    set_flag((int)*r>127?F_M:F_P);
    set_flag(*r?F_NZ:F_Z);
    set_flag(F_H);
    __parity(*r);
    set_flag(F_NN);
  }
  else /* t2 == A_PODLE_NUM */
    in_byte(a2,&A);
 }
 return 0;
}


_uchar *
f_exx (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 int a;
 
 a1=a1;t1=t1;a2=a2;t2=t2;
 if(cpu_is_in_disassemble)
  return dis(I_EXX);
 else
 {
  a=B;B=B_;B_=a;
  a=C;C=C_;C_=a;
  a=D;D=D_;D_=a;
  a=E;E=E_;E_=a;
  a=H;H=H_;H_=a;
  a=L;L=L_;L_=a;
 }
 return 0;
}


_uchar *
f_ex (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char txt[16];
 static char r1[8],r2[4];
 _uchar x,y;
 
 if(cpu_is_in_disassemble)
 {
  reg_str(r1,a1,t1);
  reg_str(r2,a2,t2);
  sprintf(txt,"%s %s,%s",dis(I_EX),r1,r2);
  return txt;
 }
 else
 {
  switch(a1)
  {
   case R_AF:       /* ex af,af' */
   x=A;A=A_;A_=x;
   x=F;F=F_;F_=x;
   break;

   case R_DE:        /* ex de,hl */
   x=D;D=H;H=x;
   x=E;E=L;L=x;
   break;

   case R_SP:         /* ex (sp),rr */
   switch(a2)
   {
    case R_HL:
    x= read_memo(SP);
    write_memo(SP,L);L=x;
    wait_tics(TICS_INTERNAL_LD);
    SP++;
    x= read_memo(SP);
    write_memo(SP,H);H=x;
    wait_tics(TICS_INTERNAL_LD);
    SP--;
    wait_tics(TICS_SP_DEC);
    break;

    case R_IX:
    x= read_memo(SP);
    write_memo(SP,IX&255);
    wait_tics(TICS_INTERNAL_LD);
    SP++;
    y= read_memo(SP);
    write_memo(SP,IX>>8);
    wait_tics(TICS_INTERNAL_LD);
    SP--;
    wait_tics(TICS_SP_DEC);
    IX=x|(y<<8);
    break;

    case R_IY:
    x= read_memo(SP);
    write_memo(SP,IY&255);
    wait_tics(TICS_INTERNAL_LD);
    SP++;
    y= read_memo(SP);
    write_memo(SP,IY>>8);
    wait_tics(TICS_INTERNAL_LD);
    SP--;
    wait_tics(TICS_SP_DEC);
    IY=x|(y<<8);
    break;
   }
   break;
  }
 }
 return 0;
}


_uchar *
f_di (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 a1=a1;t1=t1;a2=a2;t2=t2;
 if (cpu_is_in_disassemble)
   return dis(I_DI);
 else
   IFF1=IFF2=0;
 return 0;
}


_uchar *
f_ei (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 a1=a1;t1=t1;a2=a2;t2=t2;
 if (cpu_is_in_disassemble)
   return dis(I_EI);
 else
   IFF0=1;  /* IFF1=IFF2=1; */
 return 0;
}


static _uchar tbit(_uchar a1,_uchar a2)
{
   set_flag(a1&(1<<a2)?F_NZ:F_Z);
   set_flag(F_H);
   set_flag(F_NN);
   if (a2==7)
      set_flag((a1&128)?F_M:F_P);
   return 0;
}


static _uchar *
f_cb (_ushort a1,_uchar t1,_ushort a2,_uchar t2,
      char *str, _uchar (*func)(_uchar,_uchar) )
{
 static char txt[20];
 static char t[8];
 _uchar a=0;
 
 if(cpu_is_in_disassemble)
 {int i=0;
  reg_str(t,a1,t1);
  if (ddfdcb_reg)
  {  i=sprintf(txt,dis(I_LD)); reg_str(txt+i,ddfdcb_reg,A_REG);
     sprintf(txt+i+1,",");
     i += 2;
  }
  if (t2==A_EMPTY)
     sprintf(txt+i,"%s %s",str,t);
  else
     sprintf(txt+i,"%s %u,%s",str,(unsigned)a2,t);
  return txt;
 } 
 else
 {
  switch(t1)
  {
   case A_REG:
   a= *reg_ptr(a1);
   break;

   case A_PODLE_REG:
   ADDR= H<<8|L;
   goto cb_podle;

   case A_PODLE_IY_PLUS:
   ADDR= ix_iy_disp(IY,a1,1);
   goto cb_podle;
   
   case A_PODLE_IX_PLUS:
   ADDR= ix_iy_disp(IX,a1,1);
   
   cb_podle:
   a= read_memo(ADDR);
   wait_tics(TICS_LATE_ALU);
   break;

  }
  if (t2 == A_EMPTY)
     a=func(a,255);
  else
     a=func(a,a2);
  if (func != tbit)
  {
     if(t1==A_REG)
        *reg_ptr(a1)=a;
     else
        write_memo(ADDR,a);
     if (ddfdcb_reg) *reg_ptr(ddfdcb_reg)= a;
  }
  if (t2 == A_EMPTY)
  {  set_flag(F_NN);
     set_flag(F_NH);
     set_flag(a?F_NZ:F_Z);
     set_flag((a&128)?F_M:F_P);
     __parity(a);
  }
 }
 return 0;
}


_uchar *
f_bit (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_cb (a1,t1,a2,t2,dis(I_BIT),tbit);
}


static _uchar set(_uchar a1,_uchar a2)
{
   return a1|1<<a2;
}


_uchar *
f_set (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_cb (a1,t1,a2,t2,dis(I_SET),set);
}

static _uchar res(_uchar a1,_uchar a2)
{
   return a1 & ~(1<<a2);
}

_uchar *
f_res (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_cb (a1,t1,a2,t2,dis(I_RES),res);
}

static _uchar rrc(_uchar a1,_uchar a2)
{
  _uchar b=a1&1;
  a2=a2;
  set_flag(b?F_C:F_NC);
  return a1>>1|b<<7;
}

_uchar *
f_rrc (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_cb (a1,t1,a2,t2,dis(I_RRC),rrc);
}

static _uchar rlc(_uchar a1,_uchar a2)
{
  _uchar b=a1&1<<7;
  a2=a2;
  set_flag(b?F_C:F_NC);
  return a1<<1|b>>7;
}

_uchar *
f_rlc (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_cb (a1,t1,a2,t2,dis(I_RLC),rlc);
}

static _uchar rr(_uchar a1,_uchar a2)
{
  _uchar b=is_flag(F_C);
  a2=a2;
  set_flag((a1&1)?F_C:F_NC);
  return a1>>1|b<<7;
}

_uchar *
f_rr (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_cb (a1,t1,a2,t2,dis(I_RR),rr);
}

static _uchar rl(_uchar a1,_uchar a2)
{
  _uchar b=is_flag(F_C);
  a2=a2;
  set_flag((a1&1<<7)?F_C:F_NC);
  return a1<<1|b;
}

_uchar *
f_rl (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_cb (a1,t1,a2,t2,dis(I_RL),rl);
}

static _uchar sll(_uchar a1,_uchar a2)
{
  a2=a2;
  set_flag((a1&1<<7)?F_C:F_NC);
  return  a1<<1|1;
}

_uchar *
f_sll (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_cb (a1,t1,a2,t2,dis(I_SLL),sll);
}

static _uchar sla(_uchar a1,_uchar a2)
{
  a2=a2;
  set_flag((a1&1<<7)?F_C:F_NC);
  return  a1<<1;
}

_uchar *
f_sla (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_cb (a1,t1,a2,t2,dis(I_SLA),sla);
}

static _uchar srl(_uchar a1,_uchar a2)
{
  a2=a2;
  set_flag((a1&1)?F_C:F_NC);
  return a1>>1;
}

_uchar *
f_srl (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_cb (a1,t1,a2,t2,dis(I_SRL),srl);
}

static _uchar sra(_uchar a1,_uchar a2)
{
  a2=a2;
  set_flag((a1&1)?F_C:F_NC);
  return a1>>1|a1&128;
}

_uchar *
f_sra (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return  f_cb (a1,t1,a2,t2,dis(I_SRA),sra);
}


_uchar *
f_neg (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 unsigned char h;
 a1=a1;t1=t1;a2=a2;t2=t2;

 if(cpu_is_in_disassemble)
  return dis(I_NEG);
 else
 {
  h=A&15;
  A--;
  A^=255;
  set_flag(h?F_H:F_NH);
  set_flag(F_N);
  set_flag(A?F_C:F_NC);
  set_flag(A?F_NZ:F_Z);
  set_flag((A&128)?F_M:F_P);
  set_flag((A==128)?F_PE:F_PO);
 }
 return 0;
}


_uchar *
f_reti (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 a1=a1;a2=a2;t2=t2;t1=t1;
 if(cpu_is_in_disassemble)
  return dis(I_RETI);
 else
 {
  PC= read_memo(SP);
  SP++;
  PC|= read_memo(SP)<<8;
  SP++;
  IFF1=IFF2;
 }
 return 0;
}


_uchar *
f_retn (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 a1=a1;a2=a2;t2=t2;t1=t1;
 if(cpu_is_in_disassemble)
  return dis(I_RETN);
 else
 {
  PC= read_memo(SP);
  SP++;
  PC|= read_memo(SP)<<8;
  SP++;
  IFF1=IFF2;
 }
 return 0;
}


_uchar *
f_im (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 static char txt[8];
 t1=t1;a2=a2;t2=t2;

 if(cpu_is_in_disassemble)
 {
  sprintf(txt,"%s %u",dis(I_IM),(unsigned)a1);
  return txt;
 }
 else
  IM=a1;
 return 0;
}


_uchar *
f_rrd (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 _uchar a, d;
 a1=a1;t1=t1;a2=a2;t2=t2;

 if(cpu_is_in_disassemble)
  return dis(I_RRD);
 else
 {
  ADDR=H<<8|L;
  a=A;
  A&=0xf0;
  d= read_memo(ADDR);
  if (cpu_pin[busrq]) acknowledge_bus_request();
  A|=d&0x0f;
  a= d>>4 | (a&15)<<4;
  set_flag(F_NH);
  set_flag(F_NN);
  set_flag(A?F_NZ:F_Z);
  set_flag((A&128)?F_M:F_P);
  __parity(A);
  wait_tics(TICS_HALVE_SWAP);
  write_memo(ADDR,a);
 }
 return 0;
}


_uchar *
f_rld (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 _uchar a, d;
 a1=a1;t1=t1;a2=a2;t2=t2;

 if(cpu_is_in_disassemble)
  return dis(I_RLD);
 else
 {
  ADDR= H<<8|L;
  a=A;
  A&=0xf0;
  d= read_memo(ADDR);
  if (cpu_pin[busrq]) acknowledge_bus_request();
  A|=(d&0xf0)>>4;
  a= d<<4 | (a&15);
  set_flag(F_NH);
  set_flag(F_NN);
  set_flag(A?F_NZ:F_Z);
  set_flag((A&128)?F_M:F_P);
  __parity(A);
  wait_tics(TICS_HALVE_SWAP);
  write_memo(ADDR,a);
 }
 return 0;
}


static _uchar *
ld_block (_ushort a1,_uchar t1,_ushort a2,_uchar t2,
          char *str, int up, int rep)
{
 _uchar d;

 a1=a1;a2=a2;t1=t1;t2=t2;

 if(cpu_is_in_disassemble)
  return str;
 else
 {
  ADDR= H<<8|L;
  d= read_memo(ADDR);
  ADDR= D<<8|E;
  write_memo(ADDR,d);
  if (up > 0)
  { L++;H=L?H:H+1; E++;D=E?D:D+1; }
  else
  { H=L?H:H-1;L--; D=E?D:D-1;E--; }
  B=C?B:B-1;C--;
  set_flag(F_NN);
  set_flag(F_NH);
  set_flag((B|C)?F_PE:F_PO);
  wait_tics(TICS_INC_DEC_16);
  if (rep && (B|C) && !cpu_is_in_x_mode) add_to_pc(-2);
 }
 return 0;
}

_uchar *
f_ldd (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return ld_block (a1,t1,a2,t2,dis(I_LDD),-1,0);
}

_uchar *
f_lddr (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return ld_block (a1,t1,a2,t2,dis(I_LDDR),-1,1);
}


_uchar *
f_ldi (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return ld_block (a1,t1,a2,t2,dis(I_LDI),1,0);
}


_uchar *
f_ldir (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return ld_block (a1,t1,a2,t2,dis(I_LDIR),1,1);
}


static _uchar *
cp_block (_ushort a1,_uchar t1,_ushort a2,_uchar t2,
          char *str, int up, int rep)
{
 _uchar a;
 unsigned char z=0,s,h;
 
 a1=a1;a2=a2;t1=t1;t2=t2;

 if(cpu_is_in_disassemble)
  return str;
 else
 {
  ADDR= H<<8|L;
  a= read_memo(ADDR);
  if (cpu_pin[busrq]) acknowledge_bus_request();
  if (up > 0)
  { L++;H=L?H:H+1; }
  else
  { H=L?H:H-1;L--; }
  B=C?B:B-1;C--;

  h=((int)(A&15)-(a&15))<0;
  z=!(A-a);
  s=(A-a)&128;
  wait_tics(TICS_CPDI);
  
  set_flag(z?F_Z:F_NZ);
  set_flag(s?F_M:F_P);
  set_flag(h?F_H:F_NH);
  set_flag(F_N);
  set_flag((B|C)?F_PE:F_PO);
  wait_tics(TICS_INC_DEC_16);
  if (rep && (B|C) && !z && !cpu_is_in_x_mode) add_to_pc(-2);
 }
 return 0;
}

_uchar *
f_cpd (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return cp_block (a1,t1,a2,t2,dis(I_CPD),-1,0);
}

_uchar *
f_cpdr (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return cp_block (a1,t1,a2,t2,dis(I_CPDR),-1,1);
}

_uchar *
f_cpi (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return cp_block (a1,t1,a2,t2,dis(I_CPI),1,0);
}

_uchar *
f_cpir (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return cp_block (a1,t1,a2,t2,dis(I_CPIR),1,1);
}


static _uchar *
in_block (_ushort a1,_uchar t1,_ushort a2,_uchar t2,
          char *str, int up, int rep)
{
 _uchar a;

 a1=a1;a2=a2;t1=t1;t2=t2;

 if(cpu_is_in_disassemble)
  return str;
 else
 {
  in_byte(C,&a);
  ADDR= H<<8|L;
  write_memo(ADDR,a);
  if (up > 0)
  { L++;H=L?H:H+1; }
  else
  { H=L?H:H-1;L--; }
  B--;
  set_flag(F_N);
  set_flag(B&128?F_M:F_P);
  wait_tics(TICS_LATE_ALU);
  if (!B) set_flag(F_Z);
  else {set_flag(F_NZ);if(rep && !cpu_is_in_x_mode) add_to_pc(-2);}
 }
 return 0;
}

_uchar *
f_ind (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return in_block (a1,t1,a2,t2,dis(I_IND),-1,0);
}

_uchar *
f_indr (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return in_block (a1,t1,a2,t2,dis(I_INDR),-1,1);
}

_uchar *
f_ini (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return in_block (a1,t1,a2,t2,dis(I_INI),1,0);
}

_uchar *
f_inir (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return in_block (a1,t1,a2,t2,dis(I_INIR),1,1);
}


static _uchar *
out_block (_ushort a1,_uchar t1,_ushort a2,_uchar t2,
           char *str, int up, int rep)
{
 _uchar a;

 a1=a1;a2=a2;t1=t1;t2=t2;

 if(cpu_is_in_disassemble)
  return str;
 else
 {
  wait_tics(TICS_LATE_ALU);
  ADDR= H<<8|L;
  a= read_memo(ADDR);
  out_byte(C,a);
  if (up > 0)
  { L++;H=L?H:H+1; }
  else
  { H=L?H:H-1;L--; }
  B--;
  set_flag(F_N);
  set_flag(B&128?F_M:F_P);
  if (!B)set_flag(F_Z);
  else {set_flag(F_NZ);if(rep && !cpu_is_in_x_mode) add_to_pc(-2);}
 }
 return 0;
}

_uchar *
f_outd (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return out_block (a1,t1,a2,t2,dis(I_OUTD),-1,0);
}

_uchar *
f_otdr (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return out_block (a1,t1,a2,t2,dis(I_OTDR),-1,1);
}

_uchar *
f_outi (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return out_block (a1,t1,a2,t2,dis(I_OUTI),1,0);
}

_uchar *
f_otir (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
   return out_block (a1,t1,a2,t2,dis(I_OTIR),1,1);
}


_uchar *
f_nop2 (_ushort a1,_uchar t1,_ushort a2,_uchar t2)
{
 a1=a1;t1=t1;a2=a2;t2=t2;
 return  cpu_is_in_disassemble ? dis(I_ILLEGAL) : 0;
}
