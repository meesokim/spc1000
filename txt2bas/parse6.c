/****************************************************/
/* txt2bas                                          */
/* parse6.c                                         */
/* 2001.07.09. by ishioka                           */
/* 2003.05.05. by ishioka                           */
/****************************************************/

#include "txt2bas.h"
#include <math.h>

#define WORDLEN 8

#define PREFIX_OCTET	(0x0b)
#define PREFIX_HEX		(0x0c)
#define PREFIX_ADDRESS	(0x0d)
#define PREFIX_LINENUM	(0x0e)
#define PREFIX_SHORT	(0x0f)
#define PREFIX_LONG		(0x1c)
#define PREFIX_FLOAT	(0x1d)

// ------------------------------------------------------------
// variables
// ------------------------------------------------------------

static char buf[WORDLEN];	// token buffer

typedef struct _p6code6 {
	char	*str;
	int		ffflag; // has 0xff or not
	int		number;
} p6code6;

// p6 code list
#include "p6code6.h"


// ------------------------------------------------------------
// function prototype
// ------------------------------------------------------------

int findcode6(void);
double readnumber(int base, int *hasPeriod);
void writefloat(double val);


// ------------------------------------------------------------
// function definition
// ------------------------------------------------------------

// ------------------------------------------------------------
// parsemain6 : parse line
int parsemain6(void)
{
	int ret, ival, hasPeriod;
	int lineNumberFlag = 0;
	double dval;
	

	for (;;) {
		ret = findcode6();
		if (ret < 0) {

			// if no match, output 1 byte as is
			if (buf[0] == '\n') {
				// if return code, exit
				fputc(0, outfp);
				return 0;
			} else if ((isdigit(buf[0]) != 0) || (buf[0] == '.')) {
				// digit
				buf_progid(0);
				dval = readnumber(10, &hasPeriod);
				ival = (int)dval;
				if (((hasPeriod == 0) && (lineNumberFlag == 0) && (ival < 32768)) ||
					((hasPeriod == 0) && (lineNumberFlag == 1) && (ival < 65529))) {
					if (lineNumberFlag == 1) {
						fputc(PREFIX_LINENUM, outfp);
						fputc(ival & 0xff, outfp);
						fputc((ival & 0xff00) >> 8, outfp);
					} else if ((ival >= 0) && (ival <= 9)) {
						fputc(ival + 0x11, outfp);
					} else if ((ival >= 10) && (ival <= 255)) {
						fputc(PREFIX_SHORT, outfp);
						fputc(ival, outfp);
					} else {
						fputc(PREFIX_LONG, outfp);
						fputc(ival & 0xff, outfp);
						fputc((ival & 0xff00) >> 8, outfp);
					}
				} else {
					fputc(PREFIX_FLOAT, outfp);
					writefloat(dval);
				}
			} else if (strncmp(buf, "&O", 2) == 0) {
				// octed
				fputc(PREFIX_OCTET, outfp);
				buf_progid(2);
				ival = (int)readnumber(8, &hasPeriod);
				ival &= 0xffff;
				fputc(ival & 0xff, outfp);
				fputc((ival & 0xff00) >> 8, outfp);
			} else if (strncmp(buf, "&H", 2) == 0) {
				// hex
				fputc(PREFIX_HEX, outfp);
				buf_progid(2);
				ival = (int)readnumber(16, &hasPeriod);
				ival &= 0xffff;
				fputc(ival & 0xff, outfp);
				fputc((ival & 0xff00) >> 8, outfp);
			} else {
				buf_nomatch();
				if (buf[0] == '"') {
					// raw output until " or return
					int c;
					c = buf_fgetc();
					for (;;) {
						if (c == '\n') {
							buf_ungetc(1);
							break;
						}
						fputc(c, outfp);
						if (c == '"')
							break;
						c = buf_fgetc();	  
					}
					buf_match();
				} else if (isalpha(buf[0])) {
					// raw output until non-alpha-num
					// raw output until non-digit
					int c;
					c = buf_fgetc();
					for (;;) {
						//if (isalnum(c) == 0) {
						if (isdigit(c) == 0) {
							buf_ungetc(1);
							break;
						}
						fputc(c, outfp);
						c = buf_fgetc();	  
					}
					buf_match();
				}
				if ((lineNumberFlag == 1) && (buf[0] != ' ') && (buf[0] != ',')) {
					lineNumberFlag = 0;
				}
			}
		} else {
			// if match, output middle code
			buf_match();
			if ((lineNumberFlag == 1) && (p6codelist6[ret].number != CODE_MINUS)) {
				lineNumberFlag = 0;
			}
			if (p6codelist6[ret].number == CODE_ELSE) {
				fputc(':', outfp);
			} else if (p6codelist6[ret].number == CODE_QUOTE) {
				fputc(':', outfp);
				fputc(CODE_REM, outfp);
			} else if (p6codelist6[ret].ffflag == 1) {
				fputc(0xff, outfp);
			}
			fputc(p6codelist6[ret].number, outfp);
			if (p6codelist6[ret].number == CODE_DATA) {
				// raw output until return or colon
				int c, quoteflag;
				c = buf_fgetc();
				quoteflag = 0;
				for (;;) {
					if (c == '\n') {
						buf_ungetc(1);
						break;
					}
					fputc(c, outfp);
					if ((c == ':') && (quoteflag == 0))
						break;
					else if (c == '"') {
						quoteflag = 1 - quoteflag;
					}
					c = buf_fgetc();	  
				}
				buf_match();
			} else if ((p6codelist6[ret].number == CODE_REM) ||
					   (p6codelist6[ret].number == CODE_QUOTE)) {
				// raw output until return
				int c;
				c = buf_fgetc();
				while (c != '\n') {
					fputc(c, outfp);
					c = buf_fgetc();	  
				}
				buf_ungetc(1);
				buf_match();
			} else if ((p6codelist6[ret].number == CODE_AUTO   ) ||
					   (p6codelist6[ret].number == CODE_DELETE ) ||
					   (p6codelist6[ret].number == CODE_GOSUB  ) ||
					   (p6codelist6[ret].number == CODE_GOTO   ) ||
					   (p6codelist6[ret].number == CODE_THEN   ) ||
					   (p6codelist6[ret].number == CODE_ELSE   ) ||
					   (p6codelist6[ret].number == CODE_LIST   ) ||
					   (p6codelist6[ret].number == CODE_LLIST  ) ||
					   (p6codelist6[ret].number == CODE_RENUM  ) ||
					   (p6codelist6[ret].number == CODE_RESTORE) ||
					   (p6codelist6[ret].number == CODE_RESUME ) ||
					   (p6codelist6[ret].number == CODE_RUN    )) {
				// line number
				lineNumberFlag = 1;
			}
		}
	}
}

// ------------------------------------------------------------
// findcode6 : compare input and command, returns middle code or zero
int findcode6(void)
{
	int i;

	for (i = 0; i < WORDLEN; i++) {
		buf[i] = (char)toupper(buf_fgetc());
		if (buf[i] == '\0')
			break;
	}
	for (i = 0; i < sizeof(p6codelist6)/sizeof(p6code6); i++) {
		if (strncmp(buf, p6codelist6[i].str, strlen(p6codelist6[i].str)) == 0) {
			buf_progid(strlen(p6codelist6[i].str));
			return(i);
		}
	}
	return -1;
}

// ------------------------------------------------------------
// readnumber : 
double readnumber(int base, int *hasPeriod)
{
	double val;
	double basesub;
	int c;
  
	val = 0;
	c = buf_fgetc();
	while (((base == 10) && (isdigit(c) != 0)) ||
		   ((base == 16) && (isxdigit(c) != 0)) ||
		   ((base == 8) && (c >= '0') && (c <= '7'))) {
		val *= base;
		if (isdigit(c) != 0) 
			val += c - '0';
		else
			//if (c < 'a')
			if (isupper(c) != 0)
				val += c - 'A' + 10;
			else
				val += c - 'a' + 10;
		c = buf_fgetc();
	}

	if ((base == 10) && (c == '.')) {
		*hasPeriod = 1;
		c = buf_fgetc();
		basesub = 0.1;
		while (isdigit(c) != 0) {
			val += (c - '0') * basesub;
			basesub *= 0.1;
			c = buf_fgetc();
		}
	} else {
		*hasPeriod = 0;
	}

	buf_ungetc(1);

	buf_match();

	return val;
}

// ------------------------------------------------------------
// writefloat : 
void writefloat(double val)
{
	int i, exp2;
	unsigned char man[4];

	for (i = 0; i < 4; i++) {
		man[i] = 0;
	}
	if ( val == 0 ) {
		exp2 = 0;
	} else {
	val = frexp(val, &exp2);
		val = val * 2 - 1;
		//exp2++;

		for (i = 1; i < 32; i++) {
			val *= 2;
			if ( val >= 1 ) {
				man[i / 8] |= 1 << (7 - i % 8);
				val -= 1;
			}
		}
	}
	
	for (i = 0; i < 4; i++) {
		fputc(man[3 - i], outfp);
	}
	fputc(exp2 + 0x80, outfp);
}
