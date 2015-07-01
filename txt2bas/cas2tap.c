#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "console.h"

int main(int argc, char **argv) {

	int length = 0;
	char tapfile[2048];
	int c, b, d, i, dotpos, slashpos;
	FILE *IN = NULL, *OUT;
	b = 0;
	d = 0;
	if (argc < 2) {
		printf ("usage: %s cas_filename tap_filename\n", argv[0]);
		return 1;
	}
	
	IN = fopen(argv[1], "rb");
	if (!IN) {
	    printf("Could not open file %s for reading.\n", argv[1]);
		return 2;
	}
	
	if (argc > 2) 
	{
		strcpy(tapfile, argv[2]);
	}
	else
	{
		strcpy(tapfile, argv[1]);
		dotpos = 0;
		slashpos = 0;
		for(i = strlen(argv[1]); i > 0; i--)
		{
			c = *(tapfile+i);
			if (c == '.' && dotpos == 0)
				dotpos = i;
			if ((c == '\\' || c == '/') && slashpos == 0)
				slashpos = i;
		}
		if (dotpos > 0)
		{
			strcpy(tapfile+dotpos+1, "tap");
			if (slashpos > 0)
				strcpy(tapfile, tapfile+slashpos+1);
		}
		else
			strcpy(tapfile+strlen(argv[1]), "tap");		
	}
	OUT = fopen(tapfile, "wb");	
	d = 0;
	while(!feof(IN))
	{
		c = fgetc(IN);
		for(i = 0; i < 8; i++)
			fputc((c >> (7-i)) & 1 == 1 ? '1' : '0', OUT);
		//fputc(' ', OUT);
	}
	fclose(IN);
	fclose(OUT);
}

