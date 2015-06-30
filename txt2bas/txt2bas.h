/****************************************************/
/* txt2bas                                          */
/* main.h                                           */
/* 2000.03.26. by ishioka                           */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// ------------------------------------------------------------
// variables
// ------------------------------------------------------------

extern FILE *infp, *outfp;


// ------------------------------------------------------------
// function prototype
// ------------------------------------------------------------

// main.c
void t2b_exit(char *msg);

// parse.c
int parsemain5(void);
int parsemain6(void);
int parsemain1(void);

// buffer.c
int buf_fgets(void);
char buf_fgetc(void);
void buf_ungetc(int n);
void buf_match(void);
void buf_nomatch(void);
void buf_progid(int n);
