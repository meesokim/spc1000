#include "../z80-cpu.h"

typedef unsigned char(*ptr_Func)(unsigned char);

static unsigned char  p, prio = 0xff;
static ptr_Func put_byte, (*peripheral)(void);


void set_my_priority(unsigned char v, ptr_Func (*func)(void) )
{
   set_cpu_pin(inter,1);
   if (v <= prio)  { prio=v; peripheral=func; }
}


void acknowledge_interrupt(void)
{
   if (peripheral)
      put_byte= (*peripheral)();
   else
      set_cpu_pin(inter,0);
   p= prio;
   prio = 0xff;
}


unsigned char  read_request_to_peripheral(void)
{
   return  put_byte ? (*put_byte)(p) : 0xff ;
}
