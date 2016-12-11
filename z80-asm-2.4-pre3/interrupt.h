/* INTERRUPT INTERFACE */

#ifndef __INTERRUPT_H
#define __INTERRUPT_H

extern void init_interrupt_handling(void);
extern void interrupt_catcher(int signo);
extern int check_pending_interrupts(void);
extern int dump_cpu(char * filename);
extern int init_cpu(char * filename);

#endif
