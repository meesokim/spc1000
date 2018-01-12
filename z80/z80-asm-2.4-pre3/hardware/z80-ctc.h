#ifndef  __Z80_CTC_H
#define  __Z80_CTC_H

#include "../z80-global"

enum ctc_triggers { trg0, trg1, trg2, trg3, clk };
enum ctc_control_pins { ctc_rd, ctc_m1, ctc_iorq, ctc_reset};

extern int init_ctc(void);
extern void reset_ctc(void);
extern bool is_ctc_port(unsigned char port);
extern void send_pulse_to_ctc(unsigned channel);
extern void set_ctc_pin(unsigned c_pin, bit level);
extern unsigned char ctc_supplies_byte(unsigned char channel);

#endif
