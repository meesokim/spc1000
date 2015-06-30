/****************************************************/
/* txt2bas                                          */
/* parse.c                                          */
/* 2000.03.26. by ishioka                           */
/* 2000.03.27.                                      */
/****************************************************/

#include "txt2bas.h"

#define WORDLEN 8
#define CODEDATA 0x83
#define CODEREM 0x8e

// ------------------------------------------------------------
// variables
// ------------------------------------------------------------

static char buf[WORDLEN];	// token buffer

typedef struct _p6code5 {
  char	*str;
  int	number;
} p6code5;

// p6 code list
#include "p6code5.h"


// ------------------------------------------------------------
// function prototype
// ------------------------------------------------------------

int findcode5(void);


// ------------------------------------------------------------
// function definition
// ------------------------------------------------------------

// ------------------------------------------------------------
// parsemain5 : parse line
int parsemain5(void)
{
  int ret;

  for (;;) {
    ret = findcode5();
    if (ret == 0) {
      // if no match, output 1 byte as is
      if (buf[0] == '\n') {
	// if return code, exit
	fputc(0, outfp);
	return 0;
      }
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
      }
    } else {
      // if match, output middle code
      buf_match();
      fputc(ret, outfp);
      if (ret == CODEDATA) {
	// raw output until return or colon
	// debug 2000.03.27.
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
      } else if (ret == CODEREM) {
	// add 2000.03.27.
	// raw output until return
	int c;
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
      }
    }
  }
}

// ------------------------------------------------------------
// findcode5 : compare input and command, returns middle code or zero
int findcode5(void)
{
  int i;

  for (i = 0; i < WORDLEN; i++) {
    buf[i] = (char)toupper(buf_fgetc());
    if (buf[i] == '\0')
      break;
  }
  for (i = 0; i < sizeof(p6codelist5)/sizeof(p6code5); i++) {
    if (strncmp(buf, p6codelist5[i].str, strlen(p6codelist5[i].str)) == 0) {
      buf_progid(strlen(p6codelist5[i].str));
      return(p6codelist5[i].number);
    }
  }
  return 0;
}
