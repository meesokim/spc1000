#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "console.h"

int main(int argc, char **argv) {

	int length = 0;
	char casfile[2048];
	int c, b, d, i, dotpos, slashpos;
	FILE *IN = NULL, *OUT;
	b = 0;
	d = 0;
	if (argc < 2) {
		printf ("usage: %s tap_filename cas_filename\n", argv[0]);
		return 1;
	}
	
	IN = fopen(argv[1], "rb");
	if (!IN) {
	    printf("Could not open file %s for reading.\n", argv[1]);
		return 2;
	}
	
	if (argc > 2) 
	{
		strcpy(casfile, argv[2]);
	}
	else
	{
		strcpy(casfile, argv[1]);
		dotpos = 0;
		slashpos = 0;
		for(i = strlen(argv[1]); i > 0; i--)
		{
			c = *(casfile+i);
			if (c == '.' && dotpos == 0)
				dotpos = i;
			if ((c == '\\' || c == '/') && slashpos == 0)
				slashpos = i;
		}
		if (dotpos > 0)
		{
			strcpy(casfile+dotpos+1, "cas");
			if (slashpos > 0)
				strcpy(casfile, casfile+slashpos+1);
		}
		else
			strcpy(casfile+strlen(argv[1]), "cas");		
		
	}
//	printf("cas=%s, dotpos=%d\n", casfile, dotpos);
	OUT = fopen(casfile, "wb");	
	d = 0;
	while(!feof(IN))
	{
		b += ((fgetc(IN) == '1') << (7-d++));
		if (d == 8)
		{
			fputc(b, OUT);
			b = d = 0;
		}
	}
	fputc(b, OUT);
	fclose(IN);
	fclose(OUT);
}

