#include <stdio.h>
#ifdef  UNIX
#include <signal.h>
#endif

#include "z80-cpu.h"
#include "decode.h"
#include "z80-mon.h"
#ifdef  FURTHER_HARDWARE
#include "hardware/daisy_chain.h"
#endif

static unsigned long inter_mask;


int
init_cpu(char *filename)
{
  _uchar  a, buffer[8];
  unsigned  i, j;
  unsigned long t;
  static FILE *fp;
  if (filename && !(fp=fopen(filename,"r")))
     return 2;
  else if (!fp || fseek(fp,0L,SEEK_SET))
     return 3;
  fflush(fp);
  i=j=0;
  j += fread(buffer,1,8,fp);
  F=buffer[i++];
  A=buffer[i++];
  B=buffer[i++];
  C=buffer[i++];
  D=buffer[i++];
  E=buffer[i++];
  H=buffer[i++];
  L=buffer[i++];
  j += fread(buffer,1,8,fp);
  i=0;
  F_=buffer[i++];
  A_=buffer[i++];
  B_=buffer[i++];
  C_=buffer[i++];
  D_=buffer[i++];
  E_=buffer[i++];
  H_=buffer[i++];
  L_=buffer[i++];
  j += fread(buffer,1,8,fp);
  i=0;
  IX = buffer[i++];
  IX |= buffer[i++]<<8;
  IY = buffer[i++];
  IY |= buffer[i++]<<8;
  SP = buffer[i++];
  SP |= buffer[i++]<<8;
  PC = buffer[i++];
  PC |= buffer[i++]<<8;
  j += fread(buffer,1,8,fp);
  i=0;
  I=buffer[i++];
  R=buffer[i++];
  DATA=buffer[i++];
  a= buffer[i++];
  IFF0= a&1;
  IFF1= a>>1&1;
  IFF2= a>>2&1;
  set_cpu_pin(halt,a>>3&1);
  IM= a>>4 &3;
  /* set_cpu_pin(wait,a>>6&1); */
  set_cpu_pin(inter,a>>7&1);
  t = buffer[i++];
  t |= buffer[i++]<<8;
  t |= buffer[i++]<<16;
  t |= buffer[i++]<<24;
  set_tics(t);
  j += fread(&a,1,1,fp);
  set_cpu_pin(rd,a>>0&1);
  set_cpu_pin(wr,a>>1&1);
  set_cpu_pin(iorq,a>>2&1);
  set_cpu_pin(mreq,a>>3&1);
  set_cpu_pin(m1,a>>4&1);
  set_cpu_pin(rfsh,a>>5&1);
  /* set_cpu_pin(busrq,a>>6&1); */
  /* set_cpu_pin(busack,a>>7&1); */
  return j != 33;
}


int
dump_cpu(char *filename)
{
  _uchar  a, buffer[8];
  unsigned  i, j;
  static FILE *fp;
  if (filename && !(fp=fopen(filename,"w")))
     return 2;
  else if (!fp || fseek(fp,0L,SEEK_SET))
     return 3;
  i=j=0;
  buffer[i++]=F;
  buffer[i++]=A;
  buffer[i++]=B;
  buffer[i++]=C;
  buffer[i++]=D;
  buffer[i++]=E;
  buffer[i++]=H;
  buffer[i++]=L;
  j += fwrite(buffer,1,8,fp);
  i=0;
  buffer[i++]=F_;
  buffer[i++]=A_;
  buffer[i++]=B_;
  buffer[i++]=C_;
  buffer[i++]=D_;
  buffer[i++]=E_;
  buffer[i++]=H_;
  buffer[i++]=L_;
  j += fwrite(buffer,1,8,fp);
  i=0;
  buffer[i++]= IX&255;
  buffer[i++]= IX>>8;
  buffer[i++]= IY&255;
  buffer[i++]= IY>>8;
  buffer[i++]= SP&255;
  buffer[i++]= SP>>8;
  buffer[i++]= PC&255;
  buffer[i++]= PC>>8;
  j += fwrite(buffer,1,8,fp);
  a= (IFF0&1) | (IFF1&1)<<1 | (IFF2&1)<<2;
  a|= (cpu_pin[halt]&1) << 3;
  a|= (IM&3) << 4;
  a|= (cpu_pin[wait]&1) << 6;
  a|= (cpu_pin[inter]&1) << 7;
  i=0;
  buffer[i++]= I;
  buffer[i++]= R;
  buffer[i++]= DATA;
  buffer[i++]= a;
  buffer[i++]= ticks&255;
  buffer[i++]= ticks>>8&255;
  buffer[i++]= ticks>>16&255;
  buffer[i++]= ticks>>24;
  j += fwrite(buffer,1,8,fp);
  a = cpu_pin[rd]&1;
  a|= (cpu_pin[wr]&1) << 1;
  a|= (cpu_pin[iorq]&1) << 2;
  a|= (cpu_pin[mreq]&1) << 3;
  a|= (cpu_pin[m1]&1) << 4;
  a|= (cpu_pin[rfsh]&1) << 5;
  a|= (cpu_pin[busrq]&1) << 6;
  a|= (cpu_pin[busack]&1) << 7;
  j += fwrite(&a,1,1,fp);
  fflush(fp);
  return j != 33;
}


#ifdef  UNIX

void interrupt_catcher(int signo)
{
   if (signo == SIGTRAP)
      dump_cpu(".CPU");
   else if (signo == SIGUSR1)
      set_cpu_pin(reset,1);
   else if (signo == SIGUSR2)
      IFF3=1;
   else
   {  inter_mask |= (unsigned long)1<<signo;
      if (signo != SIGABRT)
#ifdef  FURTHER_HARDWARE
         set_my_priority(0xff,NULL);
#else
         set_cpu_pin(inter,1);
#endif
   }
   signal(signo,interrupt_catcher);
}


void init_interrupt_handling(void)
{
int i;
for (i=0;i<32;i++)
    signal(i, interrupt_catcher);
signal(SIGHUP, finish);
signal(SIGTERM,finish);
inter_mask=0;
/* still free no: SIG_KILL(9) , SIGSTOP(17-19), SIGCONT(17-19)
                  SIGCHLD(17-20), SIGTSTP(18-20),  0
   maybe useful simulating key board interrupts ? */
}

#else

void interrupt_catcher(int signo)
{
}


void init_interrupt_handling(void)
{
inter_mask=0;
}

#endif


/* during this routine an interrupt may occur but get not lost */
void check_pending_interrupts(void)
{
#ifdef  UNIX
  if (inter_mask & 1<<SIGABRT)
  {  inter_mask=0; cpu_is_in_disassemble=1;  }
#endif
  if (cpu_pin[inter] && IFF1)
  { unsigned long mask;
    unsigned char c;
    for (mask=1,c=0;c<32;c++,mask<<=1)
       if (inter_mask&mask)
       {  inter_mask ^= mask;
          inter_mask = 0;   /* level triggering */
          DATA=0xff;        /* code on data bus */
          inter_handler();
          break;
       }
  }
}
