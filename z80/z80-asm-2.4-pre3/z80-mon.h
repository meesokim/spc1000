#ifndef __Z80_DIS_H
#define __Z80_DIS_H

extern void print(char *str);   /* print instruction */
extern void error(int n,char *line,char* message);
extern void print_ticks(void);  /* print ticks and cycle counter */
extern void finish(int);

#endif
