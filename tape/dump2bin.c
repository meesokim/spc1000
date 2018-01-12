#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h> 
#include <io.h> 
#include <errno.h>

int getline(char **lineptr, size_t *n, FILE *stream)
{
static char line[256];
char *ptr;
unsigned int len;

   if (lineptr == NULL || n == NULL)
   {
      errno = EINVAL;
      return -1;
   }

   if (ferror (stream))
      return -1;

   if (feof(stream))
      return -1;
     
   fgets(line,256,stream);

   ptr = strchr(line,'\n');   
   if (ptr)
      *ptr = '\0';

   len = strlen(line);
#if 0  
   if ((len+1) < 256)
   {
      ptr = realloc(*lineptr, 256);
      if (ptr == NULL)
         return(-1);
      *lineptr = ptr;
      *n = 256;
   }
#endif
   //printf("%x %s\n", *lineptr, line);
   strcpy(*lineptr,line); 
   return(len);
}

int main(int argc, char **argv) 
{
	char line[256];
	char *l;
	l = line;
	if (argc < 2) 
	{
		printf("dump2bin v1.0, Copyright (C) 2018 Miso Kim\n");
		printf("Usage: dump2bin filename\n");
		exit(0);
	}
	FILE *f = fopen(argv[1], "r");
	int handle = fileno(stdout); 
	setmode(fileno(stdout), O_BINARY);
	int i = 0, read;
	size_t len;
	char buf[32];
	char hex[3];
	hex[2] = 0;
	while((read = getline (&l, &len, f)) != -1)
	{
		//printf("%s", line);
		len = 16;
		for(i = 0; i < len; i++)
		{
			memcpy(hex, line+7+i*3, 2);
			if (hex[0] < '0' && hex[1] < '0')
				break;
			//printf("%s/%d", hex, i);
			buf[i] = strtol(hex, NULL, 16) & 0xff;
		}
		write(handle, buf, i);
		memset(line, 0, 256);
	}
}