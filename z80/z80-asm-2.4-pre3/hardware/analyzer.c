#include <stdio.h>
#include "../z80-cpu.h"

static FILE *fp;
static unsigned  m1_counter;
static bool  last_m1;

static char* to_bin(unsigned value, unsigned len)
{
static  char number[33];
number[len]=0;
while (len--)
{
   number[len]= value&1 ? '1' : '0';
   value >>= 1;
}
return  number;
}

unsigned reset_analyzer(char *filename)
{
fp = fopen(filename,"wb");
return  !fp;
}

void send_pulse_to_analyzer(void)
{
fprintf(fp,"%16s(%4x)  ", to_bin(ADDRESS,16), (unsigned)ADDRESS );
fprintf(fp,"%8s(%2x)  %c%c%c%c%c ", to_bin(DATA,8), (unsigned)DATA,
    '0'+cpu_pin[rd],
    '0'+cpu_pin[wr], '0'+cpu_pin[mreq], '0'+cpu_pin[iorq], '0'+cpu_pin[m1] );
fprintf(fp," %c%c %c%c%c %c%c %c ",
    '0'+cpu_pin[busrq],'0'+cpu_pin[busack],'0'+cpu_pin[wait],'0'+cpu_pin[halt],
    '0'+cpu_pin[reset],'0'+IFF3,'0'+cpu_pin[inter],'0'+cpu_pin[rfsh]);
if (cpu_pin[m1] && !last_m1)
   m1_counter++;
last_m1=cpu_pin[m1];
fprintf(fp," %7lu %6lu %6u",ticks,cycles-1,m1_counter);
fprintf(fp,"\n");
fflush(fp);
}
