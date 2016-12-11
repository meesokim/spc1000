#include "includes"

void clock_oscillator(void)
{  /* here we can send one tic to other (not CPU) hardware */
#ifdef  Z80_CTC
   send_pulse_to_ctc(clk);
#endif
#ifdef  LOGIC_ANALYZER
   send_pulse_to_analyzer();
#endif
#ifdef  PORT_BUFFER
   send_pulse_to_port_buffer();
#endif
}


unsigned clock_frequency(void)
{
/* return  2500000;     Z80 quartz */
   return  4000000;  /* Z80A quartz */
/* return  6000000;     Z80B quartz */
/* return  8000000;     Z80H quartz */
}
