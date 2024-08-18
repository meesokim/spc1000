/****************************************************/
/* txt2bas                                          */
/* parse.c                                          */
/* 2000.03.26. by ishioka                           */
/* 2000.03.27.                                      */
/****************************************************/

#include "txt2bas.h"
#include <math.h>

#define WORDLEN 8

#define PREFIX_OCTET	(0x010)
#define PREFIX_HEX		(0x011)
#define PREFIX_ADDRESS	(0x0c)
#define PREFIX_LINENUM	(0x0b)
#define PREFIX_LONG		(0x012)
#define PREFIX_FLOAT	(0x014)

// ------------------------------------------------------------
// variables
// ------------------------------------------------------------

static char buf[WORDLEN];	// token buffer

typedef struct _spc1kcode {
  char	*str;
  int	ffflag; // has 0xff or not
  int	number;
} spc1kcode;

// spc1000 code list
#include "spc1kcode.h"


// ------------------------------------------------------------
// function prototype
// ------------------------------------------------------------

int findcode1(void);
double readnumber1(int base, int *hasPeriod);
void writefloat1(double val);

// ------------------------------------------------------------
// function definition
// ------------------------------------------------------------

// ------------------------------------------------------------
// parsemain1 : parse line
int parsemain1(void)
{
	int ret, ival, hasPeriod;
	int lineNumberFlag = 0;
	double dval;

	for (;;) {
		ret = findcode1();
		printf("ret=%d\n", ret);
		if (ret < 0) {

			// if no match, output 1 byte as is
			if (buf[0] == '\n') {
				// if return code, exit
				fputc(0, outfp);
				return 0;
			} else if ((isdigit(buf[0]) != 0) || (buf[0] == '.')) {
				// digit
				buf_progid(0);
				dval = readnumber1(10, &hasPeriod);
				ival = (int)dval;
				if (((hasPeriod == 0) && (lineNumberFlag == 0) && (ival < 32768)) ||
					((hasPeriod == 0) && (lineNumberFlag == 1) && (ival < 65529))) {
					if (lineNumberFlag == 1) {
						fputc(PREFIX_LINENUM, outfp);
						fputc(ival & 0xff, outfp);
						fputc((ival & 0xff00) >> 8, outfp);
					} else if ((ival >= 0) && (ival <= 9)) {
						fputc(ival + 0x1, outfp);
					// } else if ((ival >= 10) && (ival <= 255)) {
						// fputc(PREFIX_SHORT, outfp);
						// fputc(ival, outfp);
					} else {
						fputc(PREFIX_LONG, outfp);
						fputc(ival & 0xff, outfp);
						fputc((ival & 0xff00) >> 8, outfp);
					}
				} else {
					fputc(PREFIX_FLOAT, outfp);
					//printf("%f\n", dval);
					writefloat1(dval);
				}
			} else if (strncmp(buf, "&O", 2) == 0) {
				// octed
				fputc(PREFIX_OCTET, outfp);
				buf_progid(2);
				ival = (int)readnumber1(8, &hasPeriod);
				ival &= 0xffff;
				fputc(ival & 0xff, outfp);
				fputc((ival & 0xff00) >> 8, outfp);
			} else if (strncmp(buf, "&H", 2) == 0) {
				// hex
				fputc(PREFIX_HEX, outfp);
				buf_progid(2);
				ival = (int)readnumber1(16, &hasPeriod);
				ival &= 0xffff;
				fputc(ival & 0xff, outfp);
				fputc((ival & 0xff00) >> 8, outfp);
			} else if (buf[0] == '\'') {
				int c;
				fputc(0x3a, outfp);
				fputc(0x27, outfp);
				c = buf_fgetc();
				for (;;) {
					if (c == '\n') {
						buf_ungetc(1);
						break;
					}
					fputc(c, outfp);
					c = buf_fgetc();
				}
				buf_match();	 
			} else {
				buf_nomatch();
				if (buf[0] == '"') {
					// raw output until " or return
					int c;
					c = buf_fgetc();
					//printf("---->\"");
					for (;;) {
						if (c == '\n') {
							buf_ungetc(1);
							break;
						}
						fputc(c, outfp);
						//printf("%c", c);
						if (c == '"')
							break;
						c = buf_fgetc();	  
					}
					//printf("\"<----\n");
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
			if ((lineNumberFlag == 1) && (spc1kcodelist[ret].number != CODE_MINUS)) {
				lineNumberFlag = 0;
			}
			if (spc1kcodelist[ret].number == CODE_ELSE) {
				fputc(':', outfp);
			} else if (spc1kcodelist[ret].number == CODE_QUOTE) {
				fputc(':', outfp);
				fputc(CODE_REM, outfp);
			} else if (spc1kcodelist[ret].ffflag == 1) {
				fputc(0xff, outfp);
			}
			//printf("code=%02x\n", spc1kcodelist[ret].number);
			fputc(spc1kcodelist[ret].number, outfp);
			if (spc1kcodelist[ret].number == CODE_DATA) {
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
			} else if ((spc1kcodelist[ret].number == CODE_REM)
					   || (spc1kcodelist[ret].number == CODE_REMA)
					   || (spc1kcodelist[ret].number == CODE_QUOTE)) {
				// raw output until return
				if (spc1kcodelist[ret].number == CODE_REMA)
					fputc(0x27, outfp);
				int c;
				c = buf_fgetc();
                printf("%c",c);
				while (c != '\n') {
					fputc(c, outfp);
					c = buf_fgetc();	  
				}
				buf_ungetc(1);
				buf_match();
			} else if ((spc1kcodelist[ret].number == CODE_AUTO   ) ||
					   (spc1kcodelist[ret].number == CODE_DELETE ) ||
					   (spc1kcodelist[ret].number == CODE_GOSUB  ) ||
					   (spc1kcodelist[ret].number == CODE_GOTO   ) ||
					   (spc1kcodelist[ret].number == CODE_THEN   ) ||
					   (spc1kcodelist[ret].number == CODE_ELSE   ) ||
					   (spc1kcodelist[ret].number == CODE_LIST   ) ||
					   (spc1kcodelist[ret].number == CODE_LLIST  ) ||
					   (spc1kcodelist[ret].number == CODE_RENUM  ) ||
					   (spc1kcodelist[ret].number == CODE_RESTORE) ||
					   (spc1kcodelist[ret].number == CODE_RESUME ) ||
					   (spc1kcodelist[ret].number == CODE_RUN    )) {
				// line number
				lineNumberFlag = 1;
			}
		}
	}
}

double readnumber1(int base, int *hasPeriod)
{
	double val;
	double basesub;
	int c;
  
	val = 0;
	c = buf_fgetc();
	if (base == 10)
	{
		while(isdigit(c) != 0)
		{
			val *= 10;
			if (c > '0' && c <= '9')
				val += c - '0';
			c = buf_fgetc();
		}
		if (c == '.') {
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
	} else if (base == 16)
	{
		int valx = 0;
		while (isxdigit(c) != 0)
		{
			valx *= 16;
			if (c >= '0' && c <= '9')
				valx += c - '0';
			else if (c >= 'A' && c <= 'F')
				valx += c - 'A' + 10;
			else if (c >= 'a' && c <= 'f') 
				valx += c - 'a' + 10;
			//printf("%c, valx=%x\n", c, valx);
			c = buf_fgetc();
		}
		val = valx;
//		printf("val=%g\n", val);
	} else if (base == 8)
	{
		while(c >= '0' && c <= '7')
		{
			val *= 8;
			c = buf_fgetc();
		}
	}

	buf_ungetc(1);

	buf_match();
	return val;
}

// ------------------------------------------------------------
// writefloat : 
void writefloat1(double val)
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
		//printf("val=%f, exp2=%d ", val, exp2);
		man[0] = 0x80 + exp2;
		val = val * 2 - 1;
		for (i = 1; i < 24; i++) {
			val *= 2;
			if ( val >= 1 ) {
				man[(i+8) / 8] |= 1 << (7 - i % 8);
				val -= 1;
			}
		}
	}
	
	for (i = 0; i < 4; i++) {
		//printf("%02x ", man[i]);
		fputc(man[i], outfp);
	}
	//printf("\n");
	//fputc(exp2 + 0x80, outfp);
}

// ------------------------------------------------------------
// findcode5 : compare input and command, returns middle code or zero
int findcode1(void)
{
  int i;
  char chars[256];
  for (i = 0; i < WORDLEN; i++) {
    buf[i] = (char)toupper(buf_fgetc());
    if (buf[i] == '\0')
      break;
  }
//  scanf("%s", &chars[0]);
  for (i = 0; i < sizeof(spc1kcodelist)/sizeof(spc1kcode); i++) {
    if (strncmp(buf, spc1kcodelist[i].str, strlen(spc1kcodelist[i].str)) == 0) {
	  //printf("%s[%d]-(%d)\n", spc1kcodelist[i].str, strlen(spc1kcodelist[i].str), i);
      buf_progid(strlen(spc1kcodelist[i].str));
      return(i);
    }
  }
  return -1;
}
