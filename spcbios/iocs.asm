;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.8.0 #10562 (Linux)
;--------------------------------------------------------
	.module iocs
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _patch
	.globl _pifiles
	.globl _getchar
	.globl _putchar
	.globl _hello
	.globl _files
	.globl _getch
	.globl _gotoxy
	.globl _cas_load
	.globl _cas_save
	.globl _cls2
	.globl _cls
	.globl _attr_clear
	.globl _attr_set
	.globl _pload2
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
_ioport	=	0x0000
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_io_val:
	.ds 1
_intval:
	.ds 2
_files::
	.ds 4096
_fnum:
	.ds 1024
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area _DABS (ABS)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;iocs.c:10: void hello(void)
;	---------------------------------
; Function hello
; ---------------------------------
_hello::
;iocs.c:19: __endasm;
	ld	hl, #hello
	call	0x07F3
	ret
	hello:
	.ascii	"Recreation\n"
	.db	0
;iocs.c:20: }
	ret
;iocs.c:21: void putchar(char c) 
;	---------------------------------
; Function putchar
; ---------------------------------
_putchar::
;iocs.c:38: __endasm;
	ld	hl,#2
	add	hl,sp
	ld	a, (hl)
	cp	#0x0a
	jr	nz, next
	ld	hl, (0x11ed)
	inc	h
	xor	a
	ld	l, a
	ld	(0x11ed), hl
	ret
	next:
	call	0x0864
;iocs.c:39: }
	ret
;iocs.c:41: char getchar()
;	---------------------------------
; Function getchar
; ---------------------------------
_getchar::
;iocs.c:48: __endasm;
	push	hl
	call	0x0c62
	ld	(_io_val), a
	pop	hl
;iocs.c:49: return io_val;
	ld	iy, #_io_val
	ld	l, 0 (iy)
;iocs.c:50: }
	ret
;iocs.c:52: char getch()
;	---------------------------------
; Function getch
; ---------------------------------
_getch::
;iocs.c:66: __endasm;
	 cont:
	call	0x0c92
	push	hl
	push	af
	jr	z, cont
	ld	l, a
	ld	hl, #0xe35
	ld	a, (hl)
	ld	(_io_val), a
	pop	af
	pop	hl
;iocs.c:67: return io_val;
	ld	iy, #_io_val
	ld	l, 0 (iy)
;iocs.c:68: }
	ret
;iocs.c:70: void gotoxy(uint8 x, uint8 y)
;	---------------------------------
; Function gotoxy
; ---------------------------------
_gotoxy::
;iocs.c:82: __endasm;
	push	ix
	ld	ix,#4
	add	ix,sp
	ld	l,(ix)
	ld	h,1(ix)
	ld	(0x11ed), hl
	pop	ix
;iocs.c:83: }
	ret
;iocs.c:85: char cas_load(unsigned char *data, int len)
;	---------------------------------
; Function cas_load
; ---------------------------------
_cas_load::
;iocs.c:102: __endasm;
	push	ix
	ld	ix,#4
	add	ix,sp
	ld	l,(ix)
	ld	h,1(ix)
	ld	(0x13aa), hl
	ld	l,2(ix)
	ld	h,3(ix)
	ld	(0x13a8), hl
	call	0x0134
	ld	(_io_val), a
	pop	ix
;iocs.c:103: return io_val;
	ld	iy, #_io_val
	ld	l, 0 (iy)
;iocs.c:104: }
	ret
;iocs.c:106: void cas_save(unsigned char *data, int len)
;	---------------------------------
; Function cas_save
; ---------------------------------
_cas_save::
;iocs.c:122: __endasm;
	push	ix
	ld	ix,#4
	add	ix,sp
	ld	l,(ix)
	ld	h,1(ix)
	ld	(0x13aa), hl
	ld	l,2(ix)
	ld	h,3(ix)
	ld	(0x13a8), hl
	call	0x00b6
	pop	ix
;iocs.c:123: return;	
;iocs.c:124: }
	ret
;iocs.c:126: void cls2()
;	---------------------------------
; Function cls2
; ---------------------------------
_cls2::
;iocs.c:147: __endasm;
	ld	hl, #0x180
	ld	bc, #0x040
	ld	a, #0x20
	ld	d, a
	loope:
	out	(c), d
	push	bc
	ld	a, #0x8
	add	a, b
	ld	b, a
	xor	a
	out	(c), a
	pop	bc
	inc	bc
	dec	hl
	ld	a,h
	or	l
	jr	nz, loope
;iocs.c:148: return;
;iocs.c:149: }
	ret
;iocs.c:151: void cls()
;	---------------------------------
; Function cls
; ---------------------------------
_cls::
;iocs.c:165: __endasm;
	ld	hl, #0x200
	ld	bc, #0x000
	ld	a, #0x20
	ld	d, a
	loopf:
	out	(c), d
	inc	bc
	dec	hl
	ld	a,h
	or	l
	jr	nz, loopf
;iocs.c:166: return;
;iocs.c:167: }
	ret
;iocs.c:169: void attr_clear()
;	---------------------------------
; Function attr_clear
; ---------------------------------
_attr_clear::
;iocs.c:185: __endasm;
	ld	hl, #0x180
	ld	bc, #0x840
	di
	loopc:
	in	a, (c)
	res	0,a
	out	(c), a
	inc	bc
	dec	hl
	ld	a,h
	or	l
	jr	nz, loopc
	ei
;iocs.c:186: }
	ret
;iocs.c:188: void attr_set(char attr, int addr, int length)
;	---------------------------------
; Function attr_set
; ---------------------------------
_attr_set::
;iocs.c:211: __endasm;
	push	ix
	ld	ix,#4
	add	ix,sp
	ld	d,(ix)
	ld	c,1(ix)
	ld	b,2(ix)
	ld	l,3(ix)
	ld	h,4(ix)
	di
	loopd:
	in	a, (c)
	or	d
	out	(c), a
	inc	bc
	dec	hl
	ld	a,h
	or	l
	jr	nz, loopd
	ei
	pop	ix
;iocs.c:212: }
	ret
;iocs.c:266: void pload2(int num)
;	---------------------------------
; Function pload2
; ---------------------------------
_pload2::
;iocs.c:276: __endasm;
	push	ix
	ld	ix,#4
	add	ix,sp
	ld	l,(ix)
	ld	h,1(ix)
	call	_rpi_load
	jp	0x02C1
;iocs.c:277: }
	ret
;iocs.c:279: int pifiles(char *str)
;	---------------------------------
; Function pifiles
; ---------------------------------
_pifiles::
;iocs.c:308: __endasm;
	push	ix
	ld	ix,#4
	add	ix,sp
	ld	l,(ix)
	ld	h,1(ix)
	call	_rpi_files
	pop	ix
	ld	hl, #_data
	ld	bc, #0
	loopm:
	ld	a, (hl)
	cp	#92
	jr	nz, loopn
	ld	(hl), #0
	inc	bc
	loopn:
	inc	hl
	or	a
	jr	nz, loopm
	ld	(_intval), bc
;iocs.c:309: return intval;
	ld	hl, (_intval)
;iocs.c:310: }
	ret
;iocs.c:312: void patch(void)
;	---------------------------------
; Function patch
; ---------------------------------
_patch::
;iocs.c:465: __endasm;
	ROMPATCH:
	DI	;
	LD	SP,#00 ;
	LD	B,#0x9D ; 1. replace 7c4e --> 7c9d from address 04300h to 01500h
	LD	HL,#0x4300 ;
	L0FF0Ah:
	LD A,(HL) ;
	CP	#0x7C ;
	JR	NZ,L0FF16h ;
	DEC	HL ;
	LD	A,(HL) ;
	CP	#0x4E ;
	JR	NZ,L0FF16h ;
	LD	(HL),B ;
	L0FF16h:
	DEC HL ;
	LD	A,H ;
	CP	#0x15 ;
	JR	NC,L0FF0Ah ;
	LD	HL,#0x7A3B ; 2. put data 09dh at address 7a3bh
	LD	(HL),B ;
	LD	HL,#0x524A ; 3. font height modification
	L0FF23h:
	LD BC,#0xB
	LD	D,H ;
	LD	E,L ;
	ADD	HL,BC ;
	LD	A,H ;
	CP	#0x5B ;
	JR	C,L0FF33h ;
	LD	A,L ;
	CP	#0x4A ;
	JR	NC,L0FF44h ;
	L0FF33h:
	LD A,(DE) ;
	OR	(HL) ;
	JR	Z,L0FF3Ah ;
	INC	HL ;
	JR	L0FF23h ;
	L0FF3Ah:
	PUSH HL ;
	LD	D,H ;
	LD	E,L ;
	DEC	HL ;
	LDDR	;
	POP	HL ;
	INC	HL ;
	JR	L0FF23h ;
	L0FF44h:
	CALL	RVRTFONT ; 4. revert font '-'
	LD	DE,#0x7C4E ; 5. 7c4eh <- ff98h (size = 04fh)
	LD	HL,#PATCODE ;
	LD	BC, #51
	LDIR	;
	LD	HL,#0x1425 ;
	LD	DE,#0x7C75 ;
	CALL	PATCHCD ; 6. CALL 07c75h @ 01425h
	LD	HL,#0x4860 ;
	LD	DE,#0x7C4E ;
	CALL	PATCHCD ; 7. CALL 07c4eh @ 04860h
	LD	HL,#0x48A5 ;
	LD	DE,#0x7C5B ;
	CALL	PATCHCD ; 8. CALL 07c5bh @ 0485ah
	LD	HL,#0x05EF ;
	LD	DE,#0x7C6C ;
	CALL	PATCHCD ; 9. CALL 07c6ch @ 005efh
	LD	HL,#0x1B89 ;
	LD	DE,#0x7C90 ;
	CALL	PATCHDE ; 10. put 07c90h @ 01b89h CALL 0x1b95 -> 7c90h
	LD	HL,#0x1485 ;
	LD	DE,#0x7C89 ;
	CALL	PATCHCD ; 11. CALL 07c89h @ 01485h
	CALL	0x0056 ;
	CALL	0x4778 ;
	JP	PATCOLOR ; 12. color value patch.
	PATCHCD:
	LD	(HL),#0xCD
	INC	HL
	PATCHDE:
	LD	(HL),E
	INC	HL
	LD	(HL),D ;
	RET	;
	PATCODE:
	LD	A,E ;
	AND	#3 ;
	SRL	E ;;
	SRL	E ;;
	SLA	A ;
	SLA	A ;
	JR	L0FFB0h ;
	LD	A,E ;{
	AND	#0xC ;
	RRC	E ;
	RRC	E ;
	RRC	E ;
	RRC	E ;
	L0FFB0h:
	OR E ;
	LD	E,A ;_
	LD	A,(0x1181) ;:
	RET	;
	BIT	7,A ;
	JR	Z,L0FFBBh ;(
	CPL	;/
	L0FFBBh:
	RLCA ;
	RLCA	;
	RLCA	;
	RET	;
	PUSH	BC ;
	LD	BC,#0x01FF ;
	LD	HL,#0x7a4f ;!Oz
	L0FFC6h:
	XOR A ;
	LD	(HL),A ;w
	INC	HL ;#
	DEC	BC ;
	LD	A,B ;x
	OR	C ;
	JR	NZ,L0FFC6h ;
	LD	HL,#0x7C9D ;!|
	POP	BC ;
	RET	;
	RET	C ;
	LD	A,#0x20 ;>
	LD	L,#0 ;.
	JR	L0FFDCh ;
	LD	A,#0 ;>
	L0FFDCh:
	PUSH HL ;
	LD	HL,#0x1b51 -1 ;!P
	LD	(HL),A ;w
	POP	HL ;
	CALL	0x1b95 ;
	RET
	RVRTFONT:
	LD	HL,#0xFF ;
	LD	(#0x55D3),HL ; CHR$(&H8A)
	RET	;
	PATCOLOR:
	LD	A,#0x20 ;
	LD	HL,#0xC45 ;
	LD	(HL),A ;
	LD	HL,#0x0B22 ;
	LD	(HL),A ;
	LD	HL,#0x0ADC ;
	LD	(HL),A ;
	LD	HL,#0x0AA2 ;
	LD	(HL),A ;
	LD	HL,#0x090C ;
	LD	(HL),A ;
	EI	;
	RET	;
	PATCODEE:
;iocs.c:466: }
	ret
	.area _CODE
	.area _INITIALIZER
	.area _CABS (ABS)
