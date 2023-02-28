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
  int    ch, i;
  char buf[1];
  int handle = fileno(stdout);
  // setmode(fileno(stdout), O_BINARY);
  if (argc < 2)
     Abort ("Usage: %s bin-file size [> result]", argv[0]);

  if ((inFile = fopen(argv[1],"rb")) == NULL)
     Abort ("Cannot open %s\n", argv[1]);

  if (argc < 3) 
  {
    fseek(inFile, 0, SEEK_END);
    i = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);
    i = i + (256 - i % 256);
  } else {
    i = atoi(argv[2]);
  }
  while ((ch = fgetc(inFile)) != EOF && i > 0)
  {
	  buf[0] = ch;
    //fputc(ch, stdout);
	write(handle, buf, 1);
	i--;
  }
  buf[0] = 0;
  while (i-->0)
	write(handle, buf, 1);
  fclose (inFile);
  return (0);
}
