#include <stdio.h>
#include "console.h"

unsigned char  no_key_code, unknown_code;
unsigned char  char_map[256];
unsigned char  key_map[256];

#define  gethex(x)  \
           ('0'<=(x)&&(x)<='9'?(x):('a'<=(x)&&(x)<='f'?(x)-'a'+10:(x)-'A'+10))
#define  is_hex(x)  \
           ('0'<=(x)&&(x)<='9' || 'a'<=(x)&&(x)<='f' || 'A'<=(x)&&(x)<='F')

int init_keyboard_map(char *keyboardmapfile)
{
unsigned char  c, line[128];
unsigned  i, j;
FILE  *fp = fopen(keyboardmapfile,"r");

for (j=0;j<256;j++)
   key_map[j]= char_map[j]= j;

if (!fp)  return 1;

for(j=i=0; fgets(line,127,fp) ;i++)
{
   if (!is_hex(line[0]) || !is_hex(line[1]) || line[2]!=' ')
      continue;
   c= gethex(line[0])<<4 |gethex(line[1]);
   if (key_map[c])
      return 2;

   if (line[4]==' ')
   {  if (!j++)
         no_key_code= c;
      else if (j==2)
      {  unknown_code= c;
         for (j=0;j<256;j++)
            char_map[j]= c;
      }
   }
   else if (line[5]<=' ')
   {  if (j <= 1)  return 4;
      key_map[c]= line[4];
      char_map[key_map[c]]= c;
   }
   else if (!is_hex(line[4]) || !is_hex(line[5]) || line[6]>' ')
      return 3;
   else
   {  if (j <= 1)  return 4;
      key_map[c]= gethex(line[4])<<4 |gethex(line[5]);
      char_map[key_map[c]]= c;
   }
}
fclose(fp);
return 0;

}


void  keystrobe(unsigned char *byte)
{
   *byte= no_key_code;
   if (c_kbhit()) { *byte= char_map[c_getkey()]; }
   return;
}
