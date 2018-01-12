#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

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
  int    ch, i;

  if (argc != 2)
     Abort ("Usage: %s bin-file [> result]", argv[0]);

  if ((inFile = fopen(argv[1],"rb")) == NULL)
     Abort ("Cannot open %s\n", argv[1]);

  i = 256*80*16-35;
  while ((ch = fgetc(inFile)) != EOF)
  {
    fputc(ch, outFile);
	i--;
  }
  while (i-->0)
	fputc(0, outFile);
  fclose (inFile);
  return (0);
}
