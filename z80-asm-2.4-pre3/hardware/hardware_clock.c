#include "../z80-cpu.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define  LOWEST_PORT_NO  0x1C

int main(void)
{
   unsigned  i;
   time_t  s, t;
   FILE *fp;
   if (!(fp=fopen(Z80_PORTS,"r+b")))
   {  fprintf(stderr,"cant open Z80 ports\n");
      exit(1);
   }
   time(&s);
   while (1)
   {  unsigned char n[4];
      time(&t);
      t -= s;
      for (i=0;i<4;i++, t>>=8)
         n[i]= t & 255;
      fseek(fp,LOWEST_PORT_NO,SEEK_SET);
      fwrite(n,1,4,fp);
      fflush(fp);
      usleep(500000);
   }
   return 0;
}
