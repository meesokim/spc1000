#define  CTC_LOWEST_PORT  (0xcc&~3)

#include <stdio.h>
#include "../z80-cpu.h"
#include "daisy_chain.h"

enum ctc_triggers { trg0, trg1, trg2, trg3, clk };
enum ctc_control_pins { cs0, cs1, ctc_ce, ctc_rd, ctc_m1, ctc_iorq, ctc_iei, ctc_reset};

static bit pin[8];

#define  cs  (pin[1]<<1|pin[0])

static _uchar cbyte[4], tbyte[4], ibyte[4];
static unsigned short  counter[4];
static FILE *fp;
static unsigned long ticks_copy;


bool is_ctc_port(unsigned char port)
{
   return  port>>2 == CTC_LOWEST_PORT>>2 ;  /* 4 successive ports */
}


void reset_ctc(void)
{
int i;
   for (i=0;i<4;i++)
      cbyte[i]=0;
   for (i=0;i<8;i++)
      pin[i]=0;
}


int init_ctc(void)
{
   reset_ctc();
   return  !(fp=fopen(Z80_PORTS,"rb"));
}


unsigned char ctc_supplies_byte(unsigned char channel)
{  return  ibyte[channel&3];
}


/* function returns a pointer to a function which is */
/*    taking an unsigned char and returns an unsigned char */
/*  WOULD BE SIMPLER:  typedef unsigned char(*ptr_Func)(unsigned char);   */
/*  ptr_Func acknowledge_inter(void){...}  */
unsigned char (*
acknowledge_inter(void)
)(unsigned char)
{
   set_cpu_pin(inter,0);
   return  ctc_supplies_byte;
}


void action(unsigned channel)
{
   counter[channel]= tbyte[channel] << (cbyte[channel]&32?8:4);
   if (cbyte[channel]&128)
      set_my_priority(channel,acknowledge_inter);
   if (channel==trg2)
   {  void send_pulse_to_ctc(unsigned channel);
      send_pulse_to_ctc(trg3);  /* hardwired cascade */
   }
}


unsigned char read_word(unsigned char port)
{
   unsigned char  byte;
#ifndef  INDIRECT_CTC
   byte= DATA;
#else
   if (!fp)
      return  0xff;
   fflush(fp);
   if (fseek(fp,(long)port,SEEK_SET))
      return  0xff;  /* hardware malfunction */
   fread(&byte,1,1,fp);
#endif
   return  byte;
}


void react(unsigned char byte)
{
   if (cbyte[cs]&4)
   {
     tbyte[cs]=byte;
     cbyte[cs]&= ~4;
	 if (cbyte[cs]&8)
        cbyte[cs]|=1;
	 return;
   }
   if (byte&1) /* control word */
   {
      cbyte[cs]= byte&~3;
	  if (byte&2)
         cbyte[cs] &= ~1;
/*************
	  if (byte&4)
         4 ticks later:  in_byte(port,&tbyte[cs]);
**************/
   }
   else  /* interrupt vector supplied */
      ibyte[cs1] = (byte&~7) | cs<<1 ;
}


void send_pulse_to_ctc(unsigned channel)
{
    if (channel >= 4)
    {  int  i;
	   ticks_copy++;
       for (i=0;i<4;i++)
	      if ((cbyte[i]&1) && !(cbyte[i]&64))
             if (--counter[i] == 0)
                action(i);

    }
	else if ((cbyte[channel]&1) && (cbyte[channel]&64))
    {
       if (--counter[channel] == 0)
          action(channel);
    }
    if ((ADDRESS&255) == CTC_LOWEST_PORT+cs  &&  cpu_pin[iorq]  &&  !pin[ctc_rd])
       react(read_word(CTC_LOWEST_PORT+cs));
}


void set_ctc_pin(unsigned c_pin, bit level)
{
   pin[c_pin&7] = level;
   if (level)
      switch (c_pin)
      {
         case cs0:      break;
         case cs1:      break;
         case ctc_iei:  break;
      }
   else
      switch (c_pin)
      {
         case ctc_rd:      break;
         case ctc_m1:      break;
         case ctc_ce:      break;
   	     case ctc_iorq:    break;
         case ctc_reset:   reset_ctc(); break;
      }
}
