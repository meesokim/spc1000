#include <stdio.h>
#include "../z80-cpu.h"

int main(int argc, char *argv[])
{
unsigned port, value;
unsigned char  v;
FILE  *fp = fopen(Z80_PORTS,(argc>2? "r+": "r"));
if (!fp)
{ fprintf(stderr,"can't open %s\n",Z80_PORTS); return 1; }

if (argc <= 1)
{ printf("usage: %s port-id [new value]\n",argv[0]);  return 0; }

if (1 != sscanf(argv[1],"%2x",&port))
{ fprintf(stderr,"invalid port-id %s (must be 2 digit hexadecimal)\n",argv[1]);
  fclose(fp); return 2; }

fseek(fp,port,SEEK_SET);
if (argc == 2)
{  fread(&v,1,1,fp);
   printf("%02x\n",(unsigned)v);
}
else
{
   if (1 != sscanf(argv[2],"%2x",&value))
   { fprintf(stderr,"invalid value %s (must be 2 digit hexadecimal)\n",argv[2]);
     fclose(fp); return 3; }
   v=value;
   fwrite(&v,1,1,fp);
}

fclose(fp);
return 0;
}
