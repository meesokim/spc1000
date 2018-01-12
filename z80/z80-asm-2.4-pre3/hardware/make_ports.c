#include <stdio.h>
#include "../z80-cpu.h"

int main(void)
{
int i;
FILE  *fp = fopen(Z80_PORTS,"w+");  /* cut to zero length if exists */
if (!fp) return 1;
for (i=0;i<256;i++) fputc( 0, fp);
fclose(fp);
return 0;
}
