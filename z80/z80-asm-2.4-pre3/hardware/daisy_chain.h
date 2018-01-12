#ifndef  __DAISY_CHAIN_H
#define  __DAISY_CHAIN_H

typedef unsigned char(*ptr_Func)(unsigned char);

extern void set_my_priority(unsigned char v, ptr_Func (*func)(void) );
extern void acknowledge_interrupt(void);
extern unsigned char read_request_to_peripheral(void);

#endif
