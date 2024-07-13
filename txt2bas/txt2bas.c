/****************************************************/
/* txt2bas                                          */
/* main.c                                           */
/* 2000.03.26. by ishioka                           */
/* 2003.05.05. by ishioka                           */
/****************************************************/

#include "txt2bas.h"

typedef unsigned short word;
typedef unsigned char byte;

// ------------------------------------------------------------
// variables
// ------------------------------------------------------------

FILE *infp, *outfp;	// input/output file pointer
static int line;	// current line number
#define BASIC_START 0x7c9d

static char filename[17];

typedef struct  {
	byte type;
	char name[17];
	word size;
	word load;
	word jump;
	word protct;
	char dummy[126-26];
	word checksum;
} HEADER;

char block[sizeof(HEADER)];

// ------------------------------------------------------------
// function prototype
// ------------------------------------------------------------

void usage(void);
void getlinenumber(void);
void mk_head(void);
void mk_tail(void);
void setlinenumber(void);

int srmode = 0;
fpos_t pos = 0;
// ------------------------------------------------------------
// function definition
// ------------------------------------------------------------

// ------------------------------------------------------------
// main : txt2bas main routine
int main(int argc, char **argv)
{
	// check arguments
	if (argc == 4) {
		if (strcmp(argv[1], "-6") == 0) {
			srmode = 6;
		} else if (strcmp(argv[1], "-5") == 0) {
			srmode = 5;
		} else if (strcmp(argv[1], "-1") == 0) {
			srmode = 1;
		} else {
			usage();
		}
	} else if (argc == 3) {
		srmode = 1;
	} else {
		usage();
	}

	// open files
	if ((infp = fopen(argv[argc - 2], "r")) == NULL) {
		fprintf(stderr, "Can't open input file %s\n", argv[argc - 2]);
		exit(1);
	}
	strncpy(filename, argv[argc - 2], 17);
	if ((outfp = fopen(argv[argc - 1], "wb")) == NULL) {
		fprintf(stderr, "Can't open output file %s\n", argv[argc - 1]);
		fclose(infp);
		exit(1);
	}

	// main process
	mk_head();
	for (;;) {
		if (buf_fgets() != 0)
			break;
		getlinenumber();
		if (srmode == 5) {
			parsemain5();
		} else if (srmode == 1) {
			parsemain1();
			setlinenumber();
		} else {
			parsemain6();
		};
	}
	mk_tail();

	// close files
	fclose(infp);
	fclose(outfp);

	// exit
	return(0);
}

// ------------------------------------------------------------
// usage : 
void usage(void)
{
	fprintf(stderr, "txt2bas version 0.8 : usage\n");
	fprintf(stderr, "  txt2bas [-561] infile outfile\n");
	exit(1);
}

// ------------------------------------------------------------
// getlinenumber : get and output line number
void getlinenumber(void)
{
	int c;
	static int prevline = -1;
    char str[200];
	line = 0;
	c = buf_fgetc();
	while (isdigit(c) != 0) {
		line = line * 10 + c - '0';
		c = buf_fgetc();
	}
	if (c != ' ')
		buf_ungetc(1);
	if (prevline >= line)
    {
        sprintf(str, "Illegal line number found in %d >= %d", prevline, line);
		t2b_exit(str);
    }

	prevline = line;

	buf_match();
	// link pointer
	fgetpos(outfp, &pos);
	fputc(0xff, outfp);
	fputc(0xff, outfp);
	// line number
	fputc(line & 0x00ff, outfp);
	fputc((line & 0xff00) >> 8, outfp);
}

void setlinenumber(void)
{
	fpos_t cpos = 0;
	int len = 0;
	fgetpos(outfp, &cpos);
	fsetpos(outfp, &pos);
	len = cpos - pos;
	fputc(len & 0xff, outfp);
	fputc((len >> 8) & 0xff, outfp);
	fsetpos(outfp, &cpos);
}

// ------------------------------------------------------------
// t2b_exit : if error, output message, close files and exit
void t2b_exit(char *msg)
{
	fprintf(stderr, "%s %d\n", msg, line);
	fclose(infp);
	fclose(outfp);
	exit(1);
}

// ------------------------------------------------------------
// mk_head : make tape image header
void mk_head(void)
{
	int i;
	HEADER *head = (HEADER *) &block[0]; 

	if (srmode == 1)
	{
		strcpy(head->name, filename);
		head->load = BASIC_START;
//		fwrite(head, sizeof(HEADER), 1, outfp);
	}
	else
	{
		for (i = 0; i < 10; i++)
			fputc(0xd3, outfp);
		// filename is "t2b"
		fprintf(outfp, "t2b");
		for (i = 0; i < 3; i++)
			fputc(0x00, outfp);
	}
}

// ------------------------------------------------------------
// mk_tail : make tape image tail
void mk_tail(void)
{
	int i;

	if (srmode == 1)
	{
		fputc(0x00, outfp);
		fputc(0x00, outfp);
	}
	else
	{
		for (i = 0; i < 12; i++)
			fputc(0x00, outfp);
	}
}
