#include "iocs.h"
#include <stdio.h>

static unsigned char io_val;

void putchar(  char c) 
{
	c;
	__asm
	ld hl,#2
	add hl,sp
	ld a, (hl)
	cp #0x0a
	jr nz, next
	ld a, #0x0d
next:
	call _ACCPRT
	__endasm;
}

char getchar()
{
	__asm
	call _ASCGET
	ld (_io_val), a
	__endasm;
	return io_val;
}

char cas_load(unsigned char *data, int len)
{
	data;
	len;
	__asm
	push ix
	ld ix,#4
	add ix,sp
	ld l,(ix)
	ld h,1(ix)
	ld (MTADRS), hl
	ld l,2(ix)
	ld h,3(ix)
	ld (MTBYTE), hl
	call _MLOAD
	ld (_io_val), a
	pop ix
	__endasm;
	return io_val;
}

void cas_save(unsigned char *data, int len)
{
	data;
	len;
	__asm
	push ix
	ld ix,#4
	add ix,sp
	ld l,(ix)
	ld h,1(ix)
	ld (MTADRS), hl
	ld l,2(ix)
	ld h,3(ix)
	ld (MTBYTE), hl
	call _MSAVE
	pop ix
	__endasm;
	return;	
}

void cls()
{
	__asm
	call CLR2
	__endasm;
	return;
}

struct finfo {
	char name[13];
	int trk;
	int sec;
	unsigned char len256;
} FInfo;

