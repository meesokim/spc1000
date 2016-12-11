#include "z80-global"
#include "z80-mon.h"
#ifdef  FURTHER_HARDWARE
#include "hardware/quartz.h"
#include "hardware/includes"
#endif

_uchar F,A,B,C,D,E,H,L;
_uchar F_,A_,B_,C_,D_,E_,H_,L_;
_uchar IXl,IXh,IYl,IYh;
_uchar I,R, IM;
_ushort PC,SP,IX,IY;
_uchar DATA; /* data pins */
_ushort ADDRESS; /* address pins */
bit IFF0, IFF1,IFF2, IFF3;  /* internal EI-flipflop & interrupt flip flops */
enum cpu_control_pin { rd, wr, iorq, mreq, m1, inter, halt, wait, reset, rfsh,
                       busrq, busack };
bit cpu_pin[NO_CPU_CONTROL_PINS];


unsigned long ticks;   /* clock tick counter attached to CPU */
unsigned long cycles;  /* machine cycle counter attached to CPU */

bool cpu_is_in_disassemble;
bool cpu_is_in_x_mode;
static bool busreq_at_last_tick;


void set_tics(unsigned long t)
{  ticks=t;
   if (!t)  cycles=0;
}


void wait_tics(unsigned duration)
{
   if (duration >= 3 && !cpu_pin[busack])
      cycles++;
   while (duration)
   { /* ONE TIC OCCURS */
     busreq_at_last_tick = cpu_pin[busrq];
#ifdef  FURTHER_HARDWARE
     clock_oscillator();
#endif
     duration--;
     ticks++;
   }
}


void  set_cpu_pin(unsigned p, bit level)
{
   if (p >= NO_CPU_CONTROL_PINS || cpu_is_in_disassemble)
      return;
   cpu_pin[p]= level;
#include "hardware/system_wired"
}


void  acknowledge_bus_request(void)
{
   if (cpu_is_in_disassemble || !busreq_at_last_tick)
      return;
   while (cpu_pin[busrq])
   {  set_cpu_pin(busack,1);
#include "hardware/bus_masters"
      wait_tics(TICS_WAIT);
      print_ticks();   /** must be exported from z80-mon.c and dummy.c **/
   }
   wait_tics(TICS_WAIT);
   set_cpu_pin(busack,0);
}
