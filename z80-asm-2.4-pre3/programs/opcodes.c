#include <stdio.h>
#include "../z80-global"

#define  rela   255
#define  disp   128
#define  mid    0x3f
#define  low    0x47
#define  high   0x11

int main(void)
{
int  h, i, j;
FILE *fp=stdout;

   fprintf(fp,"%s%c%c",_Z80HEADER,'\0','\0');
   for (i=0;i<256;i++)
   {
      if (i==0xed)
      {  for (j=0;j<256;j++)
         {  fputc(i,fp),fputc(j,fp);
		    if (i < 0x80 && i > 0x40 && (i&7) == 3)
               fputc(low,fp), fputc(high,fp);
         }
      }
	  else if (i==0xcb)
      {  for (j=0;j<256;j++)
            fputc(i,fp),fputc(j,fp);
      }
	  else if (i==0xdd || i==0xfd)
      {  for (j=0;j<256;j++)
         {
            if (j==0xed || j==0xdd || j==0xfd)
               continue;
			else if (j==0xcb)
               for (h=0;h<256;h++)
                  fputc(i,fp),fputc(j,fp),fputc(disp,fp),fputc(h,fp);
            else
            {  fputc(i,fp),fputc(j,fp);
               if (j < 0xc0 && j > 0x80 && (j&7) == 6)
                  fputc(disp,fp);
			   else if (j < 0x70 && j > 0x40 && (j&7) == 6)
                  fputc(disp,fp);
			   else if (j < 0x78 && j >= 0x70 && j != 0x76)
                  fputc(disp,fp);
		       if (j < 0x40 && (j&15) == 1)
                  fputc(low,fp), fputc(high,fp);
	      	   else if (j < 0x40 && (j&7) == 6)
                  fputc(mid,fp);
		       else if (j < 0x40 && j > 0x20 && (j&7) == 2)
                  fputc(low,fp), fputc(high,fp);
	      	   else if (j > 0xc0 && (j&7) == 4)
                  fputc(low,fp), fputc(high,fp);
		       else if (j > 0xc0 && (j&7) == 6)
                  fputc(mid,fp);
		       else if (j == 0xcd)
                  fputc(low,fp), fputc(high,fp);
		       else if (j == 0xcb || j == 0xd3 || j == 0xdb)
                  fputc(mid,fp);
		       else if (j < 0x40 && j >= 0x10 && (j&7) == 0)
                  fputc(rela,fp);
            }
         }
      }
      else
      {
         fputc(i,fp);
		 if (i < 0x40 && (i&15) == 1)
            fputc(low,fp), fputc(high,fp);
		 else if (i < 0x40 && (i&7) == 6)
            fputc(mid,fp);
		 else if (i < 0x40 && i > 0x20 && (i&7) == 2)
            fputc(low,fp), fputc(high,fp);
		 else if (i > 0xc0 && (i&7) == 4)
            fputc(low,fp), fputc(high,fp);
		 else if (i > 0xc0 && (i&7) == 6)
            fputc(mid,fp);
		 else if (i == 0xcd)
            fputc(low,fp), fputc(high,fp);
		 else if (i == 0xcb || i == 0xd3 || i == 0xdb)
            fputc(mid,fp);
		 else if (i < 0x40 && i >= 0x10 && (i&7) == 0)
            fputc(rela,fp);
      }
   }
   fflush(fp);
   return 0;
}
