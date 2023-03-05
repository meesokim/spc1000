#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h> 
#include <unistd.h>
// #include <io.h> 
static void Abort (char *fmt,...)
{
  va_list args;
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  exit (1);
}

int main (int argc, char **argv)
{
  FILE  *inFile;
  FILE  *outFile = stdout;
  time_t now     = time (NULL);
  int    ch, i, w;
  char buf[1];
  if (argc < 2)
     Abort ("Usage: %s bin-file size [> result]", argv[0]);

  if ((inFile = fopen(argv[1],"ab")) == NULL)
     Abort ("Cannot open %s\n", argv[1]);

  if (argc < 3) 
    w = 256;
  else
    w = atoi(argv[2]);
  fseek(inFile, 0, SEEK_END);
  i = ftell(inFile);
  fseek(inFile, 0, SEEK_SET);
  i = (w - i % w);
  printf("filled:%d\n", i);
  buf[0] = 0;
  while (i-->0)
	fwrite(buf, 1, 1, inFile);
  fclose (inFile);
  return (0);
}
