#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../console.h"
#include "../console_token"

int main(void)
{
unsigned char  buffer[32];
unsigned a;
FILE  *fp = fopen(".CPU","r");
if (!fp)  exit(1);

c_init(BLACK);
c_cursor(C_HIDE);
do
{  
   rewind(fp);
   fread(buffer,1,32,fp);
   fflush(fp);
   c_setcolor(WHITE);
   c_goto(0,0);
   printf(" F=%02x   ", (unsigned)buffer[0]);
   c_setcolor(GREEN);
   printf("A=%02x   B=%02x   C=%0x2   D=%02x   E=%02x   H=%02x   L=%02x",
          (unsigned)buffer[1], (unsigned)buffer[2],
          (unsigned)buffer[3], (unsigned)buffer[4], (unsigned)buffer[5],
          (unsigned)buffer[6], (unsigned)buffer[7] );
   c_setcolor(WHITE);
   c_goto(0,2);
   printf("F'=%02x  ", (unsigned)buffer[8]);
   c_setcolor(GREEN);
   printf("A'=%02x  B'=%02x  C'=%0x2  D'=%02x  E'=%02x  H'=%02x  L'=%02x",
          (unsigned)buffer[9], (unsigned)buffer[10],
          (unsigned)buffer[11], (unsigned)buffer[12], (unsigned)buffer[13],
          (unsigned)buffer[14], (unsigned)buffer[15] );
   c_setcolor(BLUE);
   c_goto(0,4);
   printf("IX=%04x    IY=%04x    ",
          buffer[16]|buffer[17]<<8, buffer[18]|buffer[19]<<8 );
   c_setcolor(YELLOW);
   printf("SP=%04x    PC=%04x",
          buffer[20]|buffer[21]<<8, buffer[22]|buffer[23]<<8 );
   a= buffer[28]|buffer[29]<<8|buffer[30]<<16|buffer[31]<<24;
   c_setcolor(WHITE);
   printf("    T=%u",a);
   c_goto(0,6);
   a= (unsigned)buffer[27];
   c_setcolor(PURPLE);
   printf("I=%02x  ", (unsigned)buffer[24]);
   c_setcolor(GRAY);
   printf("R=%02x  ", (unsigned)buffer[25]);
   c_setcolor(WHITE);
   printf("DATA=%02x  ", (unsigned)buffer[26]);
   c_setcolor(CYAN);
   printf("IFF=(%u)%u%u   ", a&1, a>>1&1, a>>2&1);
   c_setcolor(RED);
   printf("%c   ", (a&8?'H':' ') );
   c_setcolor(PURPLE);
   printf("IM=%u  ", a>>4&3);
   c_setcolor(GRAY);
   printf("IO=%u  ", a>>6&1);
   c_setcolor(BRIGHT);
   printf("INT=%u", a>>7&1 );
   fflush(stdout);
   usleep(200000);
} while(!c_kbhit());
c_shutdown();
return 0;
}
