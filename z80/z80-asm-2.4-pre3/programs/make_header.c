#include <stdio.h>
#include "../z80-global"

int main(int argc, char *argv[])
{
  unsigned  start;
  if (argc <= 1)  start=0;
  else sscanf(argv[1],"%x",&start);
  fprintf(stdout,"%s%c%c",_Z80HEADER,start&255,start>>8);
  return 0;
}
