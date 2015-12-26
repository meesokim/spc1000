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
  int    ch, i, j;

  if (argc != 2)
     Abort ("Usage: %s bin-file [> result]", argv[0]);

  if ((inFile = fopen(argv[1],"rb")) == NULL)
     Abort ("Cannot open %s\n", argv[1]);

  i = 0; j = 0;
  printf("ADDR:$$ 84218421\n");
  while ((ch = fgetc(inFile)) != EOF)
  {
	  fprintf(outFile, "%04x:%02x ", j++, ch);
	  for(i = 0; i < 8; i++)
		  fprintf(outFile, "%s", ch & (0x80 >> i) ? "#" : " ");
	  fprintf(outFile, "\n");
  }
  fputc ('\n', outFile);
  fclose (inFile);
  return (0);
}
