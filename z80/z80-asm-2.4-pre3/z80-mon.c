/* MAIN PART OF MONITOR */

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "z80-cpu.h"
#include "execute.h"
#include "console_token"
#include "console.h"
#include "regs.h"
#include "file.h"
#include "asm.h"
#include "decode.h"
#include "memory.h"
#include "ports.h"
#include "interrupt.h"
#include "mini-display.h"
#include "keyboard.h"
#include "hash.h"
#include "expression.h"
#include "hardware/includes"

static _ushort MEMP;

static char *msg1="Z80 monitor V 2.3\n(c)1999-2004 Brainsoft\n";
static char *msg2="Z80 monitor (GPL)  " TIME_STAMP;
static char _string[257];
static char *string= _string+1;
static char load_name[256];
static char save_name[256];
static char proto_name[256];
static FILE *stream;

static unsigned speed;
static int keyboard_disabled;
static int current_pc;
static int follow;
static int row;     /* row for printing next instruction */
static _uchar tmp_memory[64];
static unsigned long  last_ticks;
static unsigned long  breakpt[8];

struct info{ char *label; int value; unsigned lineno; };
static struct info  *ele;
static unsigned  labels, next_label_index;

enum
{ DEFAULT,TOKEN,ADDR,  HOT,GARISH,WARN,  MEMORY,STATUS,HIDDEN };
static unsigned char color[] =
{ WHITE,  GREEN,YELLOW,RED,BRIGHT,YELLOW,BLUE,  PURPLE,GRAY   };

#define KEYBOARD_MAP_FILE "keyboard_map"


static void warning(char* message)
{
 static char txt[512];
/* c_bell(); */
 c_goto(0,23);
 c_setcolor(color[WARN]);
 sprintf(txt,"%s",message);
 c_print(txt);
 usleep(2000000L);
 c_setcolor(color[DEFAULT]);
 c_clear(0,23,79,23);
}


static void error_msg(char *line,char* message)
{
 static char txt[512];
 c_bell();
 c_goto(0,23);
 c_setcolor(color[HOT]);
 sprintf(txt,"Error: %s %s",message,line);
 c_print(txt);
 usleep(1000000L);
 c_setcolor(color[DEFAULT]);
 c_clear(0,23,79,23);
}


/*** called e.g. from decode(...) in simul.c or compile() defined in asm.c ***/
void error(int n,const char *line,char* message)
{
 static char txt[512];
 unsigned char k;
 n=n;
 c_bell();
 c_goto(0,22);
 c_print(line);
 c_goto(0,23);
 c_setcolor(color[HOT]);
 sprintf(txt,"Error: %s",message);
 c_print(txt);
 c_setcolor(color[DEFAULT]);
 c_print(" hit any key");
 k=c_getkey();
 c_clear(0,22,39,22);
 c_clear(0,23,79,23);
}


static char
__printable(char c)
{
 return (c>=32&&c<127)?c:'.';
}


/* prints instruction */
void
print(char *str)
{
 if (row >= 0)
 {
  char txt[256];
  sprintf(txt," %04x  ",current_pc);
  c_goto(0,row);
  c_setcolor(row==12?color[GARISH]:color[ADDR]);
  c_print(txt);
  c_setcolor(row==12?color[GARISH]:color[DEFAULT]);
  c_print(str);
 }
 else if (row == -1)
  fprintf(stream,"%04x %-14s",current_pc,str);
 else if (row == -2 && (MODE&16))
  ; /* first pass disassembling */
 else if (row == -2)
 { int i;
   fprintf(stream,"%04x  ",current_pc);
   for (i=0; i < (PC?PC:65536)-current_pc && i<4; i++)
      fprintf(stream,"%02x ",(unsigned)memory_at((_ushort)(current_pc+i)));
   for (;i<4;i++)
      fprintf(stream,"   ");
   for (i=0; i < (PC?PC:65536)-current_pc && i<4; i++)
   {  unsigned char  m=memory_at((_ushort)(current_pc+i));
      fprintf(stream,"%c",(m>=32&&m<127?m:' '));
   }
   for (;i<4;i++)
      fprintf(stream," ");
   if (MODE&8)  /* second pass disassembling */
   {  while (next_label_index < labels && ele[next_label_index].value < current_pc)
         next_label_index++;
      if (next_label_index >= labels || ele[next_label_index].value > current_pc)
         fprintf(stream,"        ");
      else
      { 
         fprintf(stream," %5s: ",ele[next_label_index].label);
         ele[next_label_index].label[0]=0;
         next_label_index += 1;
         while (next_label_index < labels &&
                ele[next_label_index].value == ele[next_label_index-1].value)
            ele[next_label_index].label[0]=0;
      }
   }
   fprintf(stream," %-s",str);
 }
}


static void 
__print_r16(int x,int y,char *t,int R)
{
 char txt[256];
 c_goto(x,y);
 sprintf(txt,"%s=",t);
 c_setcolor(color[TOKEN]);
 c_print(txt);
 sprintf(txt,"0x%04x %05u",R,R);
 c_setcolor(color[DEFAULT]);
 c_print(txt);
}


static void 
__print_rr(int x,int y,char *t,int R)
{
 char txt[256];
 c_goto(x,y);
 sprintf(txt,"%s=",t);
 c_setcolor(color[TOKEN]);
 c_print(txt);
 sprintf(txt,"x%02xx%02x %05u",R>>8&255,R&255,R);
 c_setcolor(color[DEFAULT]);
 c_print(txt);
}


static void
tobin(char *s,_uchar x)
{
 int a;
 for (a=0;a<8;a++)
  s[7-a]=((x&(1<<a))>>a)?'1':'0';
 s[8]=0;
}


static void 
__print_x(int x,int y,char *t,_uchar R)
{
 char txt[256];
 char b[256];
 tobin(b,R);
 c_goto(x,y);
 sprintf(txt,"%s=",t);
 c_setcolor(color[TOKEN]);
 c_print(txt);
 sprintf(txt,"x%02x (%s)",(unsigned)R,b);
 c_setcolor(color[DEFAULT]);
 c_print(txt);
}


static void 
__print_r(int x,int y,char *t,_uchar R)
{
 char txt[256];
 char b[256];
 tobin(b,R);
 c_goto(x,y);
 c_setcolor(color[TOKEN]);
 sprintf(txt,"%s=",t);
 c_print(txt);
 c_setcolor(color[DEFAULT]);
 sprintf(txt,"%03d",R);
 c_print(txt);
 c_setcolor(color[MEMORY]);
 sprintf(txt,"%c",__printable(R));
 c_print(txt);
 c_setcolor(color[DEFAULT]);
 sprintf(txt,"(%s)",b);
 c_print(txt);
}


static void
print_generell_regs(void)
{
 __print_rr(46,2,"AF",(A<<8)+F);
 __print_rr(46,3,"BC",(B<<8)+C);
 __print_rr(46,4,"DE",(D<<8)+E);
 __print_rr(46,5,"HL",(H<<8)+L);

 __print_rr(63,2,"AF'",(A_<<8)+F_);
 __print_rr(63,3,"BC'",(B_<<8)+C_);
 __print_rr(63,4,"DE'",(D_<<8)+E_);
 __print_rr(63,5,"HL'",(H_<<8)+L_);
 
 __print_r(46,12,"A",A);
 __print_r(46,13,"F",F);
 __print_r(46,14,"B",B);
 __print_r(46,15,"C",C);
 __print_r(46,16,"D",D);
 __print_r(46,17,"E",E);
 __print_r(46,18,"H",H);
 __print_r(46,19,"L",L);
 
 __print_r(63,12,"A'",A_);
 __print_r(63,13,"F'",F_);
 __print_r(63,14,"B'",B_);
 __print_r(63,15,"C'",C_);
 __print_r(63,16,"D'",D_);
 __print_r(63,17,"E'",E_);
 __print_r(63,18,"H'",H_);
 __print_r(63,19,"L'",L_);
 
}


static void
print_index_regs(void)
{
 __print_r16(56,7,"IX",IX);
 __print_r16(56,8,"IY",IY);
}
              
 
static void
print_sp_and_pc(void)
{
 __print_r16(56,9,"SP",SP);
 __print_r16(56,10,"PC",PC);
}

 
static void
print_special_regs(void)
{
 __print_x(47,21,"I",I);
 __print_x(64,21,"R",R);
}


static void
print_regs(void)
{
 print_generell_regs();
 print_index_regs();
 print_sp_and_pc();
 print_special_regs();
}


static void
print_flags(void)
{
 char txt[256];

 c_goto(45,0);
 c_setcolor(is_flag(F_C)?color[GARISH]:color[DEFAULT]);
 c_print(is_flag(F_C)?" C ":"NC ");
 c_setcolor(is_flag(F_Z)?color[GARISH]:color[DEFAULT]);
 c_print(is_flag(F_Z)?" Z ":"NZ ");
 c_setcolor(is_flag(F_M)?color[GARISH]:color[DEFAULT]);
 c_print(is_flag(F_P)?" P ":" M ");
 c_setcolor(is_flag(F_PE)?color[GARISH]:color[DEFAULT]);
 c_print(is_flag(F_PE)?"PE ":"PO ");
 c_setcolor(is_flag(F_H)?color[GARISH]:color[DEFAULT]);
 c_print(is_flag(F_H)?" H ":"NH ");
 c_setcolor(is_flag(F_N)?color[GARISH]:color[DEFAULT]);
 c_print(is_flag(F_N)?" N ":"NN ");

 c_setcolor(color[TOKEN]);
 c_goto(73,0);
 c_print("IM=");
 sprintf(txt,"%d",IM);
 c_setcolor(color[DEFAULT]);
 c_goto(76,0);
 c_print(txt);
 
 c_setcolor(color[TOKEN]);
 c_goto(65,0);
 c_print("IFF=");
 c_goto(69,0);
 c_setcolor(IFF1?color[GARISH]:color[DEFAULT]);
 c_print(IFF1?"EI":"DI");
 c_setcolor(color[DEFAULT]);
 
}


static void
print_instr(void)
{
 _ushort old_pc=PC;
 
 c_clear(7,12,22,22);
 for (row=12;row<22;row++)
 {
  current_pc=PC;
  decode(0,0);
 }
 c_goto(7,11);
 c_setcolor(color[TOKEN]);
 c_print("MNEMONIC");
 c_setcolor(color[HIDDEN]);
 if ((MODE&3)==1) c_print("d");
 else if ((MODE&3)==2)  c_print("h");
 else if ((MODE&3)==3)  c_print("x");
 else c_print(" ");
 if ((MODE&12)==4) c_print("a");
 else if ((MODE&12)==8)  c_print("l");
 else if ((MODE&12)==12)  c_print("L");
 else c_print("r");
 c_goto(0,12);
 c_setcolor(color[GARISH]);
 c_print(">");
 c_setcolor(color[DEFAULT]);
 PC=old_pc;
}


static void
print_stack(void)
{
 char txt[256];
 _ushort b,c,d;
 int r,a;

 c_goto(30,11);
 c_setcolor(color[TOKEN]);
 c_print("STACK");
 c_setcolor(color[DEFAULT]);
 for (r=12,a=SP+12;a>=SP-6;a-=2,r++)
 {
  c=a&65535;
  d=(a+1)&65535;
  b=memory_at(c)|(memory_at(d)<<8);
  sprintf(txt,"%04x  ",c);
  c_goto(24,r);
  c_setcolor(r==18?color[GARISH]:color[ADDR]);
  c_print(txt);
  c_setcolor(r==18?color[GARISH]:color[DEFAULT]);
  sprintf(txt,"%05d %04x ",b,b);
  c_print(txt);
  sprintf(txt,"%c%c",__printable(memory_at(d)),__printable(memory_at(c)));
  c_setcolor(color[MEMORY]);
  c_print(txt);
  c_setcolor(color[DEFAULT]);
 }
 c_goto(23,18);
 c_setcolor(color[GARISH]);
 c_print(">");
 c_setcolor(color[DEFAULT]);
}


static void
print_mem(void)
{
 char txt[256];
 _ushort a;
 int r,c;
 _ushort memp;

 if (follow)memp=PC;
 else memp=MEMP;

 c_goto(14,0);
 c_setcolor(color[TOKEN]);
 c_print("MEMORY");
 for (r=0,c=memp;c<memp+80;c+=8,r++)
 {_ushort b;
  a=c&65535;
  sprintf(txt,"%04x  ",a);
  c_goto(0,1+r);
  c_setcolor(color[ADDR]);
  c_print(txt);
  c_setcolor(color[DEFAULT]);
  for (b=0;b<8;b++)
     sprintf(txt+3*b,"%02x ",(unsigned)memory_at(a+b));
  c_print(txt);
  for (b=0;b<8;b++)
     sprintf(txt+b,"%c", __printable(memory_at(a+b)) );
  c_setcolor(color[MEMORY]);
  c_print(txt);
  c_setcolor(color[DEFAULT]);
 } 
}


static void
print_speed(void)
{
 c_setcolor(color[STATUS]);
 c_goto(11,23);
 if (speed)
 {  char txt[12];
    double  freq= speed*0.0025;
    if (freq < 1.0)
       sprintf(txt," %4.0f kHz ",speed*2.5);
    else if (freq < 10.0)
       sprintf(txt,"%5.3f MHz ",freq);
    else
       sprintf(txt,"%5.2f MHz ",freq);
    c_print(txt);
 }
 else
    c_print("      ");
 c_setcolor(color[DEFAULT]);
}


static void
print_status(void)
{
 c_goto(0,23);
 c_setcolor(color[STATUS]);
 c_print(follow?"FOLLOW ":"       ");
 c_goto(7,23);
 c_print(!cpu_is_in_disassemble?"RUN ":"    ");
 print_speed();
 c_setcolor(color[STATUS]);
 c_goto(20,23);
 c_print(stream?"PROTO ":"      ");
 c_setcolor(color[HOT]);
 c_goto(26,23);
 c_print(keyboard_disabled?"NOKEYS":"      ");
 c_setcolor(color[DEFAULT]);
}


void
print_ticks(void)
{
 char txt[16];
 sprintf(txt,"%10lu",ticks);
 c_setcolor(color[DEFAULT]);
 c_goto(41,10);
 c_print(txt);
 c_setcolor(cpu_pin[busack]?color[HOT]:color[TOKEN]);
 c_print(" T");
 last_ticks=ticks;
 sprintf(txt,"%10lu",cycles);
 c_setcolor(color[DEFAULT]);
 c_goto(41,8);
 c_print(txt);
 c_setcolor(color[TOKEN]);
 c_print(" M");
}


static void
print_breaks(void)
{
 int i,j;
 char txt[16];
 for (j=i=0;i<8;i++)
    if (breakpt[i]>>16)
    {
      sprintf(txt,"%4lu",breakpt[i]>>16);
      c_setcolor(color[HOT]);
      c_goto(10*j,22);
      c_print(txt);
      sprintf(txt,"%04x",(unsigned)breakpt[i]&65535);
      c_setcolor(color[ADDR]);
      c_print(txt);
      j++;
      c_setcolor(color[DEFAULT]);
    }
}


static void print_halt(void)
{
   c_setcolor(color[HOT]);
   c_goto(78,0);
   c_print(cpu_pin[halt]?"H":" ");
   c_setcolor(color[DEFAULT]);
}


/* draw screen */
static void
print_panel(void)
{
 print_regs();
 print_flags();
 print_instr();
 print_stack();
 print_mem();
 print_status();
 print_ticks();
 print_breaks();
 print_halt();
}


static void
show_keys(void)
{
 int i=0;
 c_cls();
#include  "help_layout"
 c_goto(0,23);
 c_print("PRESS ANY KEY TO QUIT HELP.");
 c_goto(79-strlen(msg2),23);
 c_print(msg2);
 c_getkey();
 c_cls();
}


/* clear all general purpos registers and IX, IY, and SP */
static void
clear_user_regs(void)
{
 A=0;B=0;C=0;D=0;E=0;F=0;H=0;L=0;
 A_=0;B_=0;C_=0;D_=0;E_=0;F_=0;H_=0;L_=0;
 SP=0;IX=0;IY=0;
}


/* put SP to 0xFFFC and store 4 bytes starting at 0xFFFC */
static void
stack_halt(void)
{
 _ushort d;
 SP=0xfffc;
 d=SP+2;
 write_memo(SP, d&255);
 write_memo(SP+1, d>>8);
 write_memo(SP+2,0xf3); /* DI */
 write_memo(SP+3,0x76); /* HALT */
}


static void
ask_flag(void)
{
 unsigned char c;
 c_clear(0,23,79,23);
 c_goto(0,23);
 c_print("Toggle flag: Zero, Carry, Parity, Sign, H, N?");
 c=c_getkey();
 switch(c)
 {
  case 'z':
  case 'Z':
  set_flag(is_flag(F_Z)?F_NZ:F_Z);
  break;

  case 'c':
  case 'C':
  set_flag(is_flag(F_C)?F_NC:F_C);
  break;

  case 'p':
  case 'P':
  set_flag(is_flag(F_PE)?F_PO:F_PE);
  break;

  case 's':
  case 'S':
  set_flag(is_flag(F_M)?F_P:F_M);
  break;

  case 'h':
  case 'H':
  set_flag(is_flag(F_H)?F_NH:F_H);
  break;

  case 'n':
  case 'N':
  set_flag(is_flag(F_N)?F_NN:F_N);
  break;

  default:
  c_bell();
  break;
 }
 c_clear(0,23,79,23);
}


/* reads string from input (will be stored in pointer), writes prompt message */
/* string cannot be longer than max_len */
/* return value: 0=ok, 1=escape pressed */
static int
ask_str(char *pointer,char *message,int max_len)
{
 unsigned char c;
 int a=strlen(pointer),l=strlen(message);
 
 c_clear(0,23,79,23);
 c_cursor(C_NORMAL);
 c_goto(0,23);
 c_setcolor(color[GARISH]);
 c_print(message);
 c_setcolor(color[DEFAULT]);
 c_print(pointer);
 while(1)
 {
  c=c_getkey();
  if (c==K_ESCAPE)
  {
   pointer[0]=0;
   c_clear(0,23,79,23);
   c_cursor(C_HIDE);
   return 1;
  }
  if (c==K_ENTER)
  {
   c_clear(0,23,79,23);
   c_cursor(C_HIDE);
   return 0;
  }
  if (c==K_BACKSPACE&&a)
  {
   a--;
   pointer[a]=0;
   c_clear(l+strlen(pointer),23,79,23);
   c_goto(l+strlen(pointer),23);
   continue;
  }
  if (c>=32&&a<max_len)
  {
   pointer[a]=c;   /* do charu se dava unsigned char */
   a++;
   pointer[a]=0;
   c_goto(l,23);
   c_print(pointer);
   c_clear(l+strlen(pointer),23,79,23);
   c_goto(l+strlen(pointer),23);
  }
 }
}


/* tries to convert a string to an unsigned number */
/* on error returns ~0 */
static unsigned  convert_to_uns(char *txt)
{
 unsigned  i, j, val;
 for (i=0;txt[i]==' '||txt[i]=='\t';i++);
 if (!txt[i] || txt[i]=='+' || txt[i]=='-')  return ~0;
 j=test_number(txt+i,&val); /* Single character-representation '?' is allowed */
 if (!j)  return ~0;
 return val;
}


/* reads a non negative integer from input */
/* on escape or error returns ~0 */
static unsigned
ask(char *str,unsigned init_val)
{
 static char txt[256];
 
 if (init_val)sprintf(txt,"%u",init_val);
 else txt[0]=0;
 if (ask_str(txt,str,16))return ~0;
 return  convert_to_uns(txt);
}


/* reads a non negative integer from input */
/* on escape or error returns ~0 */
static unsigned
ask_x(char *str,unsigned init_val)
{
 static char txt[256];
 
 if (init_val)sprintf(txt,"0x%x",init_val);
 else txt[0]=0;
 if (ask_str(txt,str,16))return ~0;
 return  convert_to_uns(txt);
}


static void 
ask_general_16_register(char *prompt, _uchar *high, _uchar *low)
{ unsigned x;
  x=ask(prompt,*high<<8|*low);
  if (x>65535)
  { c_bell();print_status(); }
  else
  { *high= x>>8;
    *low= x&255;
    cpu_is_in_disassemble=1;
  }
}


static void 
ask_special_16_register(char *prompt, _ushort *reg16)
{ unsigned x;
  x=ask(prompt,*reg16);
  if (x>65535)
  { c_bell();print_status(); }
  else
  { *reg16= x;
    cpu_is_in_disassemble=1;
  }
}


static void
ask_16bit_register(void)
{
 unsigned char c;
 c_clear(0,23,79,23);
 do {
    c_goto(0,23);
    c_print("Toggle Register: bc, de, hl, BC, DE, HL, xX, yY, Sp");
    c=c_getkey();
    switch(c)
    {  
       case 'b':  ask_general_16_register("BC=",&B,&C);
                  break;

       case 'B':  ask_general_16_register("BC'=",&B_,&C_);
                  break; 

       case 'd':  ask_general_16_register("DE=",&D,&E);
                  break;

       case 'D':  ask_general_16_register("DE'=",&D_,&E_);
                  break;

       case 'h':  ask_general_16_register("HL=",&H,&L);
                  break;

       case 'H':  ask_general_16_register("HL'=",&H_,&L_);
                  break;

       case 'x': case 'X':
                  ask_special_16_register("IX=",&IX);
                  break;

       case 'y': case 'Y':
                  ask_special_16_register("IY=",&IY);
                  break;

       case 's': case 'S':
                  ask_special_16_register("SP=",&SP);
                  break;

       case K_ENTER:
                  break;

       default:   c_bell();
                  break;
    }
    print_regs();
 } while (c != K_ENTER);
 c_clear(0,23,79,23);
}


static void 
ask_8bit_register(char *prompt, _uchar *reg8)
{
  unsigned x=ask(prompt,*reg8);
  if (x>255)
  { c_bell();print_status(); }
  else
  { *reg8= x;
    cpu_is_in_disassemble=1;
  }
}


static void
ask_register(void)
{
 unsigned char c;
 c_clear(0,23,79,23);
 do {
    c_goto(0,23);
    c_print("Toggle Register: a, f, b, c, d, e, h, l, "
                             "A, F, B, C, D, E, H, L, =, I, R");
    c=c_getkey();
    switch(c)
    {  
       case '=':  ask_16bit_register();
                  break;

       case 'a':  ask_8bit_register("A=",&A);
                  break;

       case 'A':  ask_8bit_register("A'=",&A_);
                  break;

       case 'f':  ask_8bit_register("F=",&F);
                  print_flags();
                  break;

       case 'F':  ask_8bit_register("F'=",&F_);
                  print_flags();
                  break;

       case 'b':  ask_8bit_register("B=",&B);
                  break;

       case 'B':  ask_8bit_register("B'=",&B_);
                  break;

       case 'c':  ask_8bit_register("C=",&C);
                  break;

       case 'C':  ask_8bit_register("C'=",&C_);
                  break;

       case 'd':  ask_8bit_register("D=",&D);
                  break;

       case 'D':  ask_8bit_register("D'=",&D_);
                  break;

       case 'e':  ask_8bit_register("E=",&E);
                  break;

       case 'E':  ask_8bit_register("E'=",&E_);
                  break;

       case 'h':  ask_8bit_register("H=",&H);
                  break;

       case 'H':  ask_8bit_register("H'=",&H_);
                  break;

       case 'l':  ask_8bit_register("L=",&L);
                  break;

       case 'L':  ask_8bit_register("L'=",&L_);
                  break;

       case 'I':  ask_8bit_register("I=",&I);
                  break;

       case 'R':  ask_8bit_register("R=",&R);
                  break;

       case K_ENTER:
                  break;

       default:   c_bell();
                  break;
    }
    print_regs();
 } while (c != K_ENTER);
 c_clear(0,23,79,23);
}


static void
protocol(void)
{
  row = -1;
  current_pc=PC;
  decode(0,0);
  fprintf(stream,"%c%c%c%c ",(is_flag(F_C)?'C':'.'),(is_flag(F_Z)?'Z':'.')
                            ,(is_flag(F_M)?'-':'+'),(is_flag(F_PE)?'e':'o'));
  fprintf(stream,"%02x %02x %02x %02x %02x %02x %02x ",A,B,C,D,E,H,L);
  fprintf(stream,"%02x %02x %02x %02x %02x %02x %02x ",A_,B_,C_,D_,E_,H_,L_);
  fprintf(stream,"%04x %04x %04x\n",IX,IY,SP);
  fflush(stream);
  PC=current_pc;
}


void finish(int signo)
{
   c_shutdown();
   exit(-signo);
}


static int compare_addr(const struct info *left, const struct info *right)
{
   return  left->value == right->value ? 0 : left->value < right->value ? -1:1;
}


/*-------------------------------MAIN-----------------------------------------*/
int
main(int argc,char **argv)
{
 unsigned char c, rom_path[256], bank_mapping_descr[128], emu=0;
 unsigned short old_pc;
 int b,a,s;
 unsigned short start;
 unsigned x;

 string[-1]=' ';  /* that is ok! (prevents any label recognation in compile) */
 strcpy(rom_path,".");
 strcpy(bank_mapping_descr,"");

 for (b=s=1;s<argc && *argv[s]=='-';s++,b=1)
 {
    if (!*(argv[s]+b))
       continue;
    else if (*(argv[s]+b)=='E')
       emu=1;
    else if (*(argv[s]+b)=='B')
    {  if (s+1>=argc || 1!=sscanf(argv[s+1],"%127s",bank_mapping_descr))
          fprintf(stderr,"Error: option -B needs a filename argument\n");
       else
          b=0, s++;
    }
    else if (*(argv[s]+b)=='R')
    {  if (s+1>=argc || 1!=sscanf(argv[s+1],"%255s",rom_path))
          fprintf(stderr,"Error: option -R needs a path argument\n");
       else
          b=0, s++;
    }
    else if (*(argv[s]+b)=='h' || *(argv[s]+b)=='?')
    {
       printf("%s\n",msg1);
       printf("Usage: z80-mon [-h] [-E] [-R path] [<filename> ...]\n");
       return 0;
    }
 }
 clear_memory();
 MEMP=0;

 for (b=s;b<argc;b++)
 {
  stream=fopen(argv[b],"rb");
  if (!stream){fprintf(stderr,"Error: Can't read file \"%s\".\n",argv[b]);return(1);}
  if (read_header(stream,&start,&x)){fprintf(stderr,"Error: \"%s\" is not a Z80 ASM file.\n",argv[b]);return(1);}
  dma_write(start,x,stream);
  MEMP=start;
  fclose(stream);
  stream=0;
 }

 c_init(BLACK); /** for DEBUG set it to WHITE **/
 c_cursor(C_HIDE);
 define_scroll_line(39,23,40);
 init_keyboard_map(KEYBOARD_MAP_FILE);
 if(init_ports())
    warning("asynchron buffered CPU port access impossible");
 else if (*bank_mapping_descr)
    init_banks(rom_path,bank_mapping_descr);
 if (emu)
 {  init_cpu(".CPU");
    cpu_is_in_disassemble=0;
    speed= 1<<12;
    keyboard_disabled=1;
    follow=0;
 }
 else
 {
#ifdef Z80_CTC
    if (init_ctc())
       error_msg("asynchron CTC port access denied","system:");
#endif
#ifdef LOGIC_ANALYZER
    reset_analyzer(".bus_proto");
#endif
    reset_cpu();
    follow=1;
 }
 last_ticks=0;
 io_address=NULL;

 disable_pseudo=0;
 init_interrupt_handling();
 print_panel();

 while (1)
 {
  if (cpu_pin[halt] && !IFF1)
  { if (!cpu_is_in_disassemble && emu)
    {  dump_cpu(".CPU");
       break;
    }
    else
       cpu_is_in_disassemble= 1;
  }
  if (cpu_pin[busrq])
     /* don't run CPU */ ;
  else if (cpu_is_in_disassemble)
     usleep(20000);
  else
  {
   for (a=0;a<8;a++)
      if ((breakpt[a]&65535) == PC)
         if (breakpt[a]>>16)
         {  breakpt[a] -= 65536;
            print_breaks();
            break;
         }
   if (a == 8 || breakpt[a]>>16)
   { if (stream)protocol();
     decode(0,1);  /* here we decode and execute the current opcode */
     if (cpu_is_in_disassemble || speed <= 1) print_panel();
     else if (speed==4)
     {print_regs();print_ticks();print_halt();print_flags();}
     else
     { if (ticks>=last_ticks+speed)
       print_sp_and_pc(),print_ticks(),print_halt();
     }
   }
   else
   { cpu_is_in_disassemble=1; print_panel();
   }
  }
  if (cpu_pin[busrq])
    c=no_key_code; /* no keyboard input possible */
  else
  {
    if (cpu_is_in_disassemble)  keyboard_disabled=0;
    c = !keyboard_disabled && c_kbhit() ? c_getkey() : no_key_code;
    if (cpu_is_in_disassemble && c != no_key_code)  set_cpu_pin(halt,0);
  }
  if (c != no_key_code)
  switch(c)
  {
   case '?':
   case 'h':
   case 'H':
   show_keys();
   print_panel();
   break;
   
   /* case 'q':  too dangerous */
   case 'Q': /* quit */
   c_shutdown();
   return 0;

   case 'x':
   case 'X':
   string[0]=0;
   ask_str(string,"Execute instruction: ",40);
   cpu_is_in_x_mode= 1;
   io_address=tmp_memory;
   set_cpu_pin(iorq,1);
   old_pc=PC;
   set_start_address(0);
   disable_pseudo=1;
   if (!compile(string-1))
   {  disable_pseudo=0;
      PC=0;
      decode(&old_pc,1);
   }
   else
      disable_pseudo=0;
   set_cpu_pin(iorq,0);
   io_address=NULL;
   cpu_is_in_x_mode= 0;
   print_panel();
   break;
   
   case ' ':  /* move to next instruction */
   decode(0,2);
   print_panel();
   break;

   case K_BACKSPACE:  /* exec one instruction */
   if (!cpu_is_in_disassemble) break;
   if (stream)protocol();
   decode(0,1);
   print_panel();
   break;

   case 'f': /* toggle flag */
   case 'F':
   ask_flag();
   print_panel();
   break;
   
   case '=':
   ask_register();
   break;

   case 's': /* change SP */
   x=ask_x("SP=",SP);
   if (x>65535){c_bell();print_status();break;}
   cpu_is_in_disassemble=1;
   SP=x;
   print_panel();
   break;
   
   case '+': /* speed */
   if (!speed)  speed = 1;
   else if (speed < 15000)  speed <<=2;
   print_speed();
   break;

   case '-': /* speed */
   if (speed > 1)  speed >>= 2;
   else if (cpu_is_in_disassemble) speed =0;
   print_speed();
   break;

   case 'r': /* toggle cpu_wait/run */
   case 'R':
   if (!cpu_pin[halt] || c=='R') cpu_is_in_disassemble= !cpu_is_in_disassemble;
   if (!cpu_is_in_disassemble && !speed) speed=1;
   print_panel();
   break;

   case 'i': /* change IM */
   case 'I':
   IM++;
   IM%=3;
   print_panel();
   break;

   case 't': /* toggle disassembling numbers */
   /* 0:  default,  1: decimal, 2: hexadecimal, 3: hexadecimal with prefix */
   MODE= (MODE&~3) | ((MODE&3)+1 &3);
   print_panel();
   break;

   case 'j': /* toggle disassembling of address displacements (jr/djnz) */
   /* 0:  relative, 4: absolute 4 digit hexadecimal, 8,12: K-labels,V-labels */
   MODE= (MODE&~12) | ((MODE&12)+4 &12);
   print_panel();
   break;

   case '.': /* enter instruction */
   set_start_address(MEMP);
   do {
   sprintf(string,"0x%04x: ",MEMP);
   if (ask_str(string,"Put instruction into memory: ",40))break;
   compile(string+7);
   MEMP=get_current_address();
   print_panel();
   } while (string[0]);
   break;
   
   case K_ENTER: /* enter instruction */
   string[0]=0;
   ask_str(string,"Put instruction at PC: ",40);
   set_start_address(PC);
   compile(string-1);
   print_panel();
   break;
   
   case '^': /* toggle EI/DI */
   IFF2=IFF1;
   IFF1^=1;
   print_panel();
   break;

   case '*':
   clear_user_regs();
   reset_banks();

   case '@':
#ifdef Z80_CTC
   reset_ctc();
#endif
   reset_cpu();
   last_ticks=0;
   print_panel();
   break;

   case '#':
   clear_memory();
   print_panel();
   break;

   case '&': /* init stack_halt */
   stack_halt();
   print_panel();
   break;

   case '$':
   last_ticks=0;
   set_tics(0);
   print_ticks();
   break;

   case 'p': /* ask about address */
   case 'P':
   x=ask_x("PC=",PC);
   if (x>65535){c_bell();print_status();break;}
   cpu_is_in_disassemble=1;
   PC=x;
   print_panel();
   break;

   case 'u':  /* defm */
   case 'U':
   sprintf(string,"0x%04x: defm \"",MEMP);
   if (ask_str(string+13,string,100))break;
   string[strlen(string)]='"';
   set_start_address(MEMP);
   compile(string+7);
   print_panel();
   break;
   
   case 'w':  /* defw */
   case 'W':
   sprintf(string,"0x%04x: defw ",MEMP);
   if (ask_str(string+12,string,64))break;
   set_start_address(MEMP);
   compile(string+7);
   print_panel();
   break;
   
   case 'v':  /* defb */
   case 'V':
   sprintf(string,"0x%04x: defb ",MEMP);
   if (ask_str(string+12,string,64))break;
   set_start_address(MEMP);
   compile(string+7);
   print_panel();
   break;
   
   case K_TAB:
   follow^=1;
   print_panel();
   break;
   
   case 'm': /* ask about start of memory dump */
   case 'M':
   follow=0;
   x=ask_x("Enter memory address: ",MEMP);
   if (x>65535){c_bell();break;}
   MEMP=x;
   print_panel();
   break;

   case 'L':   /* load */
   if (ask_str(load_name,"Load file: ",40)){c_bell();print_status();break;}
   stream=fopen(load_name,"rb");
   if (!stream){stream=0;error_msg(load_name,"Can't read file");print_status();break;}
   if (read_header(stream,&start,&x))
   {warning("Not a Z80 ASM file"); start=ask("Load to address: ",PC);}
   else
   { sprintf(string,"starting at 0x%x  %u bytes put",a,x); warning(string); }
   dma_write(start,x,stream);
   fclose(stream);
   stream=0;
   print_panel();
   break;

   case 'S':   /* save */
   if (ask_str(save_name,"Save as: ",40)){c_bell();break;}
   x=ask("Length: ",0);
   if (PC+x>65536)break;
   stream=fopen(save_name,"wb");
   if (!stream){stream=0;error_msg(save_name,"Can't write to file ");print_status();break;}
   write_header(stream,PC);
   dma_read(PC,x,stream);
   fclose(stream);
   stream=0;
   print_panel();
   break;

   case 'D': /* dump/disassemble */
   if (ask_str(save_name,"Disassemble to: ",40)){c_bell();break;}
   sprintf(string,"0x%04x - ",PC);

   x=ask_x(string,0);
   if (x>65536 || x < PC)
   {error_msg("","memory wrap_around");print_status();break;}
   stream=fopen(save_name,"a");
   if (!stream)
      {stream=0;error_msg(save_name,"Can't append to file");print_status();break;}
   row = -2;
   if (MODE&8)
   {  hash_table_init();
      MODE |= 16;
   }
   old_pc=PC;
   while (PC <= x)
   {
     current_pc=PC;
     decode(0,0);
     if (!(MODE&16))
       fprintf(stream,"\n");
     if (PC < current_pc)  break;
   }
   PC=old_pc;
   if (MODE&16)
   {  char blanks[32];
      for (a=0;a<31;a++)
         blanks[a]=' ';
      blanks[a]=0;
      MODE &= ~16;
      if (labels=table_entries())
      {  ele= malloc(labels*sizeof(struct info));
         for(a=0;next_table_entry(&ele[a].label,&ele[a].value,&ele[a].lineno);a++);
         qsort(ele,labels,sizeof(struct info),compare_addr);
      }
      next_label_index=0;
      old_pc=PC;
      fprintf(stream,"%sORG 0x%04x\n",blanks,old_pc);
      while (PC <= x)
      {
        current_pc=PC;
        decode(0,0);
        fprintf(stream,"\n");
        if (PC < current_pc)  break;
      }
      PC=old_pc;
   }
   if (MODE&8)
   { 
     char blanks[23];
     b=0;
     for (a=0;(unsigned)a<labels;a++)
        if (ele[a].label[0]) b++;
     if (labels)
        fprintf(stream,";\n; %u Used Labels    %u Undefined Labels:\n",labels-b,b);
     if ((unsigned)b<labels)
     {  for (a=0;a<23;a++)
           blanks[a]=' ';
        blanks[a]=0;
     }
     for (a=0;(unsigned)a<labels;a++)
        if (ele[a].label[0])
           fprintf(stream,"%s%5s   EQU  0x%04hx\n",blanks,ele[a].label,ele[a].value);
     free(ele);
     free_hash_table();
   }
   fclose(stream);
   stream=0;
   print_panel();
   break;

   case '\"': /* protocol execution */
   if (ask_str(proto_name,"Protocol into: ",40)){c_bell();break;}
   stream=fopen(proto_name,"a");
   if (!stream){stream=0;error_msg(proto_name,"Can't append to file");print_status();break;}
   fprintf(stream," PC  mnemonic     flags A  B  C  D  E  H  L "
                  " A' B' C' D' E' H' L'  IX   IY   SP\n");
   print_status();
   break;

   case '!':  /* toggle keyboard interrupt */
   keyboard_disabled^=1;
   print_status();
   break;

   case K_ESCAPE:  /* NMI */
   nmi_handler();
   print_panel();
   break;

   case '%': /* ask about break point */
   for (a=0;a<8;a++)
      if ((breakpt[a] & 0xffff) == MEMP)
         break;
   if (a==8)
   for (a=0;a<8;a++)
      if ((breakpt[a]>>16 & 0xffff) == 0)
         break;
   if (a==8)  {c_bell();break;}
   x=ask_x("Enter breakpoint count: ",breakpt[a]>>16);
   if (x>65535){c_bell();break;}
   breakpt[a] = MEMP | x<<16;
   print_breaks();
   break;

   MEMP=x;
   print_panel();
   break;

   default:
   c_bell();
   break;
  }
  if (cpu_is_in_disassemble)  emu=0;  /* really finish emulation mode ?? */
  if (IFF0)
     IFF2=IFF1=1;
  else if (cpu_pin[reset])
     reset_cpu();
  else if (cpu_pin[busrq])
     /* don't serve interrupts */ ;
  else if (IFF3)
     nmi_handler();
  else
     check_pending_interrupts();
 }
 c_shutdown();
 return 0;
}
