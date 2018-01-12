#ifndef __SIMUL_H
#define __SIMUL_H

extern _ushort ddfdcb_reg;  /* indicates register of an FDCB or DDCB instruction */

extern void refresh_cycle(void);
extern void decode(_ushort *store_pc, int exec);
extern void inter_handler(void);
extern void nmi_handler(void);
extern void reset_cpu(void);

#endif
