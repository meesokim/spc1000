#include <stdio.h>
#include "console.h"
#include "console_token"
#define  CLEAR  '\n'
#define  HOME   '\r'
#define  LEFT   '\b'
#define  RIGHT  '\t'

/***  definition for mini display used inside port.c ***/
int  display_size=0, display_xpos, display_ypos;

static int  col_pos=0;
static unsigned char  display[256+1];

void define_scroll_line(unsigned x, unsigned y, unsigned len)
{
   display_xpos=x;
   display_ypos=y;
   display_size=len;
}

void display_in_line(unsigned char  byte)
{
   if (byte >= 32)
   {
      if (!display_size)  return;
      display[col_pos&255] = byte;
      col_pos++;
      c_setcolor(D_CYAN);
      if (col_pos < display_size)
      {  c_goto(display_xpos+col_pos,display_ypos);
         putchar((int)byte);
      }
      else
      {  display[col_pos&255] = '\0';
         c_goto(display_xpos,display_ypos);
         fputs(display+(col_pos+1-display_size&255),stdout);
         if ((col_pos+1-display_size&255) >= (col_pos&255))
            fputs(display,stdout);
      }
   }
   else if (byte == RIGHT)
      col_pos++;
   else if (byte == LEFT)
      col_pos--;
   else if (byte == HOME)
      col_pos=0;
   else if (byte == CLEAR)
   {  int i;
      col_pos=0;
      for (i=0;i<display_size;i++)
         display[i]=' ';
      if (!display_size)  return;
      c_goto(display_xpos,display_ypos);
      fputs(display,stdout);
   }
}
