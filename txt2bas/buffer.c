/****************************************************/
/* txt2bas                                          */
/* buffer.c                                         */
/* 2000.03.26. by ishioka                           */
/* 2000.05.25., 2001.01.06.                         */
/****************************************************/

#include "txt2bas.h"

#define BUFSIZE 1024

// ------------------------------------------------------------
// variables
// ------------------------------------------------------------

static char buf[BUFSIZE];	// line buffer
static int bufid_read;		// index of reading buffer
static int bufid_write;		// index of writing buffer

// sjis <-> p6 chatacter set table
#include "sjis.h"

extern int srmode;

// ------------------------------------------------------------
// function prototype
// ------------------------------------------------------------

void buf_conv(void);


// ------------------------------------------------------------
// function definition
// ------------------------------------------------------------

// ------------------------------------------------------------
// buf_fgets : like fgets
int buf_fgets(void)
{
	bufid_read = 0;
	bufid_write = 0;
	if (fgets(buf, BUFSIZE, infp) == NULL) 
		return 1;
	else {
		if (srmode != 1)
			buf_conv();
		return 0;
	}
}

// ------------------------------------------------------------
// buf_fgetc : like fgetc
char buf_fgetc(void)
{
	if (bufid_read < 1024)
		return(buf[bufid_read++]);
	return 0;
}

// ------------------------------------------------------------
// buf_ungetc : like ungetc
void buf_ungetc(int n)
{
	bufid_read -= n;
}

// ------------------------------------------------------------
// buf_match : if token matchs p6code, forward write index
void buf_match(void)
{
	bufid_write = bufid_read;
}

// ------------------------------------------------------------
// buf_nomatch : if token doesn't match p6code, forward only 1
void buf_nomatch(void)
{
	fputc(toupper(buf[bufid_write++]), outfp);
	bufid_read = bufid_write;
}

// ------------------------------------------------------------
// buf_progid : forward write index and backward read index
void buf_progid(int n)
{
	bufid_write += n;
	bufid_read = bufid_write;
}

// ------------------------------------------------------------
// buf_conv : convert 2 bytes character to 1 byte character
void buf_conv(void)
{
	int i1, i2, j, c, opt;

	i1 = i2 = 0;
	while ((c = (unsigned char)buf[i1++]) != '\0') {
		if ((c >= 0x80) && (c <= 0x9f)) {
			c = c * 256 + (unsigned char)buf[i1++];

			// for dakuon/han-dakuon 2000.05.25.
			opt = 0;
			for (j = 0; j < sizeof(sjistbl2)/sizeof(int); j++) {
				if (sjistbl2[j] == c) {
					if (j == 50) {
						// for "vu" (2001.01.06)
						opt = 1;
						c = 0x8345;
					} else {
						opt = (j < 40) ? 1 : 2;
						c = c - opt;
						break;
					}
				}
			}

			for (j = 0; j < 256; j++) {
				if (sjistbl[j] == c) {
					if (j <= 0x1f) {
						// if graphic character
						buf[i2++] = (char)0x14;
						buf[i2++] = (char)(j + 0x30);
					} else {
						buf[i2++] = (char)j;
						if (opt != 0)
							buf[i2++] = (char)(0xde + opt - 1);
					}
					break;
				}
			}
			if (j == 256)
				t2b_exit("Illegal character found in next of");
		} else
			buf[i2++] = (char)c;
	}
	buf[i2] = '\0';
}
