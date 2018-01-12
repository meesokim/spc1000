#include "../z80-cpu.h"
#include "zds_token"

static unsigned _m1_counter = 0;

void set_ssl_trigger(void)
{
   _m1_counter = 3;
}

void reset_ssl_trigger(void)
{
   _m1_counter = 0;
}

void trigger_ssl(void)
{
   if (_m1_counter > 0)
      if (--_m1_counter == 0)
         IFF3 = 1;
}

bool is_ssl_port(unsigned char port)
{
   return  port == SSL_PORT ;
}
