#include "../z80-cpu.h"
#include "../console.h"
#include "../ports.h"
#include <stdio.h>
#include <unistd.h>

#define  COLUMNS  20
#define  ROWS      4
#define  BYTES_PER_CHAR  6 /* character is represented by a 8x6 pixel array */

#define  CTRL_PORT   0x0a
#define  DATA_PORT   0x0b
/***  interpretation of the control_byte
Bit 0:   valid byte on DATA_PORT (set by sender)
         byte received on DATA_PORT and ready for next (reset by receiver)
Bit 1&2:  00 byte is to display
          01 byte indicates x-pos,
		  10 byte indicates y-pos
          11 byte is mode
Bit 3:   LCD-display ready to receive data
Bit 7:   switched by sender each new byte on DATA_PORT (no latency display)

mode byte:   0   set draw mode
mode byte:   1   xor draw mode
mode byte:   2   or draw mode
mode byte:   3   and draw mode

mode byte:   0   no scrolling
mode byte:   4   scroll display one pixel row up
mode byte:   8   scroll display one pixel row down
mode byte:  12   scroll display one pixel col left
mode byte:  16   scroll display one pixel col right
mode byte:  20   scroll display one line up
mode byte:  24   scroll display one line down

mode byte:   0   ignore draw mode setting
mode byte:  32   invert total display
mode byte:  64   clear total display
mode byte:  96   refresh total display
mode byte: 128   grafic cursor home
***/

static unsigned char  pixel[BYTES_PER_CHAR*COLUMNS][ROWS];
static unsigned  sub_x=0, cur_x=0, cur_y=0, draw_mode;

int write_8pixel(unsigned char data)
{
   int i;
   if (cur_x >= COLUMNS || cur_y >= ROWS)  return 1;
   switch (draw_mode)
   {  
       case 0:  pixel[6*cur_x+sub_x][cur_y]= data;  break;
       case 1:  pixel[6*cur_x+sub_x][cur_y]^= data;  break;
       case 2:  pixel[6*cur_x+sub_x][cur_y]|= data;  break;
       case 3:  pixel[6*cur_x+sub_x][cur_y]&= data;  break;
   }
   if (draw_mode)
      data = pixel[6*cur_x+sub_x][cur_y];
   for (i=0;i<8;i++,data>>=1)
   {  c_goto(2*(6*cur_x+sub_x),8*cur_y+i);
      printf((data&1) ? "@ ":". ");
   }
   fflush(stdout);
   sub_x++;
   if (sub_x == 6)  cur_x++, sub_x=0;
   return 0;
}


void refresh(void)
{  unsigned  i, j;
   for (j=0;j<ROWS*8;j++)
      for (c_goto(0,j),i=0;i<COLUMNS*6;i++)
         printf("%c ", pixel[i][j>>3]&1<<(j&7) ? '@' : '.');
   fflush(stdout);
}


int main(void)
{
   unsigned char  contr, data, i, j;
   if (init_ports())
      return 1;
   c_init(0);
   for (j=0;j<ROWS*8;j++)
      for (c_goto(0,j),i=0;i<COLUMNS*6;i++)
      {  printf(". ");  if (!(j&7)) pixel[i][j>>3]=0;  }
   fflush(stdout);
   in_byte(CTRL_PORT,&contr);
   contr |= 8;
   while (1)
   {
      out_byte(CTRL_PORT,contr);
      do {
         in_byte(CTRL_PORT,&contr);
		 if (c_kbhit())  goto end;
         usleep(20000);
      } while(!(contr&1));
      in_byte(DATA_PORT,&data);
      if ((contr&6) == 0)
         write_8pixel(data);
      else if ((contr&6) == 2)
         cur_x=data;
      else if ((contr&6) == 4)
         cur_y=data;
	  else
      {
         if (data&128)  cur_x=cur_y=0;
         data &= 127;
         switch (data>>2)
         {
             case 5:
                      for (j=0;j<ROWS;j++)
                      for (i=0;i<COLUMNS*6;i++)
                         pixel[i][j] = (j+1==ROWS ? 0 : pixel[i][j+1]);
					  refresh();
					  break;
             case 6:
                      for (j=0;j<ROWS;j++)
                      for (i=0;i<COLUMNS*6;i++)
                         pixel[i][j] = (j ? 0 : pixel[i][j-1]);
					  refresh();
					  break;
             case 8: 
                      for (j=0;j<ROWS*8;j++)
                      for (c_goto(0,j),i=0;i<COLUMNS*6;i++)
                      {  if (!(j&7)) pixel[i][j>>3]^=0xff;   
                         printf("%c ", pixel[i][j>>3]&1<<(j&7) ? '@' : '.');  }
                      fflush(stdout);
					  break;
             case 16:
                      for (j=0;j<ROWS*8;j++)
                      for (c_goto(0,j),i=0;i<COLUMNS*6;i++)
                      {  printf(". ");  if (!(j&7)) pixel[i][j>>3]=0;  }
                      fflush(stdout);
					  break;
         }
		 if (data>>5 != 0)
		    draw_mode = data&3;
      }
      contr ^= 1;
   }
end:  c_shutdown();
   return 0;
}
