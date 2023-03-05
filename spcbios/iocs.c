#include "iocs.h"

static unsigned char io_val;
static int intval;
static int fnum[512];
extern char data[];

void hello(void)
{
	__asm
	ld hl, #hello
	call DEPRT
	ret
hello:
	.ascii "Recreation\n"
	.db 0
	__endasm;
}

void putchar(char c) 
{
	c;
	__asm
	ld hl,#2
	add hl,sp
	ld a, (hl)
	cp #0x0a
	jr nz, next
	ld hl, (CURXY)
	inc h
	xor a
	ld l, a
	ld (CURXY), hl
	ret
next:
	call _ACCPRT
	__endasm;
}

char getchar()
{
	__asm
	cont0:
	call _ASCGET
	or a
	jr z, cont0
	ld l, a
	__endasm;
}

char getch()
{
	__asm
	cont:
	push hl
	push af
	call _KEYGT
	jr z, cont
	ld l, a
	ld hl, #0xe35
	ld a, (hl)
	ld (_io_val), a
	pop af
	pop hl
	__endasm;
	return io_val;
}

void gotoxy(uint8 x, uint8 y)
{
	x;
	y;
	__asm
	push ix
	ld ix,#4
	add ix,sp
	ld l,(ix)
	ld h,1(ix)
	ld (CURXY), hl
	pop ix
	__endasm;
}

void cls2()
{
	__asm
	ld hl, #0x180
	ld bc, #0x040
	ld a, #0x20
	ld d, a
loope:
	out (c), d
	push bc
	ld a, #0x8
	add a, b
	ld b, a
	xor a
	out (c), a
	pop bc
	inc bc
	dec hl
	ld a,h
	or l
	jr nz, loope
	__endasm;
	return;
}

void cls()
{
	__asm
	ld hl, #0x200
	ld bc, #0x000
	ld a, #0x20
	ld d, a
loopf:
	out (c), d
	inc bc
	dec hl
	ld a,h
	or l
	jr nz, loopf
	__endasm;
	return;
}

void attr_clear()
{
	__asm
	ld hl, #0x180
	ld bc, #0x840
	di
loopc:
	in a, (c)
	res 0,a
	out (c), a
	inc bc
	dec hl
	ld a,h
	or l
	jr nz, loopc
	ei
	__endasm;
}

void attr_set(char attr, int addr, int length)
{
	__asm
	push ix
	ld ix,#4
	add ix,sp
	ld d,(ix)
	ld c,1(ix)
	ld b,2(ix)
	ld l,3(ix)
	ld h,4(ix)
	di
loopd:
	in a, (c)
	or d
	out (c), a
	inc bc
	dec hl
	ld a,h
	or l
	jr nz, loopd
	ei
	pop ix
	__endasm;
}

// void pload2(int num)
// {
// 	num;
// 	__asm
// 	push ix
// 	ld ix,#4
// 	add ix,sp	
// 	ld l,(ix)
// 	ld h,1(ix)
// 	call _rpi_load	
// 	jp PLOAD_FAKE 
// 	__endasm;
// }

int pifiles(char *fl)
{
	__asm
RPI_FILES	= 0x20
RPI_LOAD	= 0x21
RPI_SAVE	= 0x22
RPI_OLDNUM	= 0x23
sendcmd  = 0xCB62 
senddata = 0xCB6a
recvdata = 0xCB99
	push ix
	ld ix,#4
	add ix,sp
	ld l,(ix)
	ld h,1(ix)
	// call _rpi_files
	ld d, #RPI_FILES
	call sendcmd
loop:
	ld d, (hl)
	call senddata
	inc hl
	ld a, d
	or a
	jr nz, loop
	ld hl, #_data
	ld b, a
	ld c, a
loop2:
	call recvdata
	ld a, d
	or a
	jr z, next2
	cp #92
	jr nz, next0
	inc bc
	ld a, #0
	jr next1
next0:
	cp #0x5f
	jr nz, next1
	ld a, #45
next1:	
	ld (hl), a
	inc hl
	jr loop2
next2:
	ld (hl), a
	pop ix	
	push bc 
	pop hl 
// 	ld bc, #0
// loopm:	
// 	ld a, (hl)
// 	cp #92
// 	jr nz, loopn
// 	ld (hl), #0
// 	inc bc
// loopn:
// 	inc hl
// 	or a
// 	jr nz, loopm
// loopq:
// 	inc bc
// 	push bc
// 	pop hl
// 	push hl
// check:
// 	ld a,(hl)
// 	or a
// 	jr z, end0
// 	cp #0x5f
// 	jr nz, next0
// 	ld a, #32
// 	ld (hl), a
// next0:
// 	inc hl 
// 	jr check
// end0:
// 	pop hl
	__endasm;
	// return intval;
}

int pioldnum() 
{
	__asm;
	push de
	push bc
	ld d, #RPI_OLDNUM
	call sendcmd
	call recvdata
	ld l, d
	call recvdata
	ld h, d
	pop bc
	pop de
	__endasm;
}

// void patch(void)
// {
// 	__asm
// ROMPATCH:	
//     DI                	;
//     LD	SP,#00      	;
//     LD	B,#0x9D         	; 1. replace 7c4e --> 7c9d from address 04300h to 01500h  
//     LD	HL,#0x4300      	;
// L0FF0Ah:   	LD	A,(HL)  ;
//     CP	#0x7C           	;
//     JR	NZ,L0FF16h     	; 
//     DEC	HL            	;
//     LD	A,(HL)         	;
//     CP	#0x4E           	;
//     JR	NZ,L0FF16h     	; 
//     LD	(HL),B         	;
// L0FF16h:   	DEC	HL      ;
//     LD	A,H            	;
//     CP	#0x15           	;
//     JR	NC,L0FF0Ah     	;
//     LD	HL,#0x7A3B     	; 2. put data 09dh at address 7a3bh
//     LD	(HL),B         	;
//     LD	HL,#0x524A      	; 3. font height modification
// L0FF23h:   	LD	BC,#0xB      	
//     LD	D,H            	; 
//     LD	E,L            	;
//     ADD	HL,BC         	;
//     LD	A,H            	;
//     CP	#0x5B           	;
//     JR	C,L0FF33h      	;
//     LD	A,L            	;
//     CP	#0x4A           	;
//     JR	NC,L0FF44h     	;
// L0FF33h:   	LD	A,(DE)  ;
//     OR	(HL)           	;
//     JR	Z,L0FF3Ah      	;
//     INC	HL            	;
//     JR	L0FF23h        	;
// L0FF3Ah:   	PUSH	HL  ;
//     LD	D,H            	;
//     LD	E,L            	;
//     DEC	HL            	;
//     LDDR              	;
//     POP	HL            	;
//     INC	HL            	;
//     JR	L0FF23h        	;
// L0FF44h:   	
//     CALL	RVRTFONT    ; 4. revert font '-' 
//     LD	DE,#0x7C4E      	; 5. 7c4eh <- ff98h (size = 04fh)
//     LD	HL,#PATCODE     	;
//     LD	BC, #51
//     LDIR              	;
//     LD	HL,#0x1425      	; 
//     LD	DE,#0x7C75      	;
//     CALL	PATCHCD     ; 6. CALL 07c75h @ 01425h
//     LD	HL,#0x4860      	; 
//     LD	DE,#0x7C4E      	;
//     CALL	PATCHCD     ; 7. CALL 07c4eh @ 04860h
//     LD	HL,#0x48A5      	; 
//     LD	DE,#0x7C5B      	;
//     CALL	PATCHCD     ; 8. CALL 07c5bh @ 0485ah
//     LD	HL,#0x05EF      	; 
//     LD	DE,#0x7C6C      	;
//     CALL	PATCHCD     ; 9. CALL 07c6ch @ 005efh
//     LD	HL,#0x1B89      	;
//     LD	DE,#0x7C90      	;
//     CALL	PATCHDE     ; 10. put 07c90h @ 01b89h CALL GRAPH -> 7c90h
//     LD	HL,#0x1485      	;
//     LD	DE,#0x7C89      	;
//     CALL	PATCHCD     ; 11. CALL 07c89h @ 01485h 
//     CALL	INITSB      ;
//     CALL	BEEP      	;
//     JP	PATCOLOR       	; 12. color value patch.
// PATCHCD:	
// 	LD	(HL),#0xCD
// 	INC	HL    
// PATCHDE:	
// 	LD	(HL),E
//     INC	HL            	
//     LD	(HL),D         	;
//     RET               	;
// PATCODE:
//     LD	A,E            	;	
//     AND	#3          	;
//     SRL	E             	;;
//     SRL	E             	;;
//     SLA	A             	;
//     SLA	A             	;
//     JR	L0FFB0h        	;
//     LD	A,E            	;{
//     AND	#0xC          	;
//     RRC	E             	;
//     RRC	E             	;
//     RRC	E             	;
//     RRC	E             	;
// L0FFB0h:   	OR	E              	;
//     LD	E,A            	;_
//     LD	A,(GCOLOR)     	;:
//     RET               	;

//     BIT	7,A           	;
//     JR	Z,L0FFBBh      	;(
//     CPL               	;/
// L0FFBBh:   	RLCA              	;
//     RLCA              	;
//     RLCA              	;
//     RET               	;

//     PUSH	BC           	;
//     LD	BC,#0x01FF      	;
//     LD	HL,#KEYBUF      	;!Oz
// L0FFC6h:   	XOR	A             	;
//     LD	(HL),A         	;w
//     INC	HL            	;#
//     DEC	BC            	;
//     LD	A,B            	;x
//     OR	C              	;
//     JR	NZ,L0FFC6h     	; 
//     LD	HL,#0x7C9D      	;!|
//     POP	BC            	;
//     RET               	;

//     RET	C             	;
//     LD	A,#0x20         	;> 
//     LD	L,#0         	;.
//     JR	L0FFDCh        	;
//     LD	A,#0         	;>
// L0FFDCh:   	PUSH	HL           	;
//     LD	HL,#CLS1LP-1   	;!P
//     LD	(HL),A         	;w
//     POP	HL            	;
//     CALL	GRAPH      	;
//     RET      
// RVRTFONT:	
//     LD	HL,#0xFF      	;
//     LD	(#0x55D3),HL    	; CHR$(&H8A)
//     RET               	;

// PATCOLOR:	
//     LD	A,#0x20         	;
//     LD	HL,#0xC45      	;
//     LD	(HL),A         	;
// 	LD	HL,#0x0B22      	;
//     LD	(HL),A         	;
//    	LD	HL,#0x0ADC      	;
//     LD	(HL),A         	; 
//    	LD	HL,#0x0AA2      	;
//     LD	(HL),A         	;
//    	LD	HL,#0x090C      	;
//     LD	(HL),A         	;
//     EI                	;
//     RET	           	;
// PATCODEE:	
// 	__endasm;
// }


