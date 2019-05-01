;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.6.0 #9615 (MINGW64)
;--------------------------------------------------------
	.module main
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _main
	.globl _pioldnum
	.globl _pifiles
	.globl _gotoxy
	.globl _cls2
	.globl _cls
	.globl _pload2
	.globl _attr_set
	.globl _attr_clear
	.globl _strlen
	.globl _getchar
	.globl _printf
	.globl _data
	.globl _run
	.globl _listdir
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
_ioport	=	0x0000
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_data::
	.ds 4096
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
;main.c:15: void main(void)
;	---------------------------------
; Function main
; ---------------------------------
_main::
	push	ix
	ld	ix,#0
	add	ix,sp
	push	af
	dec	sp
;main.c:20: char *param = "SD:/\\*.tap";
;main.c:24: memset(data, 0, 0x800);
	ld	hl,#_data
	ld	(hl), #0x00
	ld	e, l
	ld	d, h
	inc	de
	ld	bc, #0x07ff
	ldir
;main.c:25: cls();
	call	_cls
;main.c:26: printf(" RPI extension box for SPC1000\n-------------------------------");
	ld	hl,#___str_1
	push	hl
	call	_printf
;main.c:28: t = pifiles(param);
	ld	hl, #___str_0
	ex	(sp),hl
	call	_pifiles
	pop	af
;main.c:29: l = t / pg;
	ld	bc,#0x000c
	push	bc
	push	hl
	call	__divsint
	pop	af
	pop	af
	ld	c,l
;main.c:30: num = pioldnum();
	push	bc
	call	_pioldnum
	pop	bc
	ld	b,l
;main.c:31: p = num / pg;
	push	bc
	ld	a,#0x0c
	push	af
	inc	sp
	push	bc
	inc	sp
	call	__divuchar
	pop	af
	pop	bc
	ld	-3 (ix),l
;main.c:32: c = num % pg;
	push	bc
	ld	a,#0x0c
	push	af
	inc	sp
	push	bc
	inc	sp
	call	__moduchar
	pop	af
	ld	e,l
	pop	bc
;main.c:33: while(1)
00108$:
;main.c:35: gotoxy(0,2);
	push	bc
	push	de
	ld	hl,#0x0200
	push	hl
	call	_gotoxy
	pop	af
	pop	de
	push	de
	ld	d,#0x0c
	push	de
	ld	a,-3 (ix)
	push	af
	inc	sp
	call	_listdir
	inc	sp
	ld	hl,#___str_2
	ex	(sp),hl
	call	_printf
	pop	af
	pop	de
	pop	bc
;main.c:38: gotoxy(4,c+2);
	ld	b,e
	inc	b
	inc	b
	push	bc
	push	de
	push	bc
	inc	sp
	ld	a,#0x04
	push	af
	inc	sp
	call	_gotoxy
	pop	af
	call	_getchar
	pop	de
	pop	bc
;main.c:52: c = (pg-1 > c ? c + 1: pg-1);
	ld	-2 (ix),e
	ld	-1 (ix),#0x00
;main.c:40: switch (ch)
	ld	a,l
	cp	a,#0x0d
	jr	Z,00105$
	cp	a,#0x1c
	jr	Z,00102$
	cp	a,#0x1d
	jr	Z,00101$
	cp	a,#0x1e
	jr	Z,00103$
	sub	a, #0x1f
	jr	Z,00104$
	jr	00108$
;main.c:42: case 0x1d:
00101$:
;main.c:43: p = (p > 0 ? p - 1: 0);
	ld	a,-3 (ix)
	or	a, a
	jr	Z,00112$
	ld	b,-3 (ix)
	dec	b
	jr	00113$
00112$:
	ld	b,#0x00
00113$:
	ld	-3 (ix),b
;main.c:44: break;
	jr	00108$
;main.c:45: case 0x1c:
00102$:
;main.c:46: p = (p < l ? p + 1: l);
	ld	a,-3 (ix)
	sub	a, c
	jr	NC,00114$
	ld	b,-3 (ix)
	inc	b
	jr	00115$
00114$:
	ld	b,c
00115$:
	ld	-3 (ix),b
;main.c:47: break;
	jp	00108$
;main.c:48: case 0x1e:
00103$:
;main.c:49: c = (c > 0 ? c - 1: 0);
	ld	a,e
	or	a, a
	jr	Z,00116$
	dec	e
	jp	00108$
00116$:
	ld	e,#0x00
;main.c:50: break;
	jp	00108$
;main.c:51: case 0x1f:
00104$:
;main.c:52: c = (pg-1 > c ? c + 1: pg-1);
	ld	a,-2 (ix)
	sub	a, #0x0b
	ld	a,-1 (ix)
	rla
	ccf
	rra
	sbc	a, #0x80
	jr	NC,00118$
	inc	e
	jp	00108$
00118$:
	ld	e,#0x0b
;main.c:53: break;
	jp	00108$
;main.c:54: case 0x0d:
00105$:
;main.c:55: run((int)p * pg + c);
	ld	l,-3 (ix)
	ld	h,#0x00
	push	de
	ld	e, l
	ld	d, h
	add	hl, hl
	add	hl, de
	add	hl, hl
	add	hl, hl
	pop	de
	ld	a,l
	add	a, -2 (ix)
	ld	l,a
	ld	a,h
	adc	a, -1 (ix)
	ld	h,a
	push	bc
	push	de
	push	hl
	call	_run
	pop	af
	pop	de
	pop	bc
;main.c:57: }
	jp	00108$
___str_0:
	.ascii "SD:/"
	.db 0x5c
	.ascii "*.tap"
	.db 0x00
___str_1:
	.ascii " RPI extension box for SPC1000"
	.db 0x0a
	.ascii "-----------------------------"
	.ascii "--"
	.db 0x00
___str_2:
	.ascii "-------------------------------       RETURN for Execution"
	.db 0x00
;main.c:61: void run(int num)
;	---------------------------------
; Function run
; ---------------------------------
_run::
	push	ix
	ld	ix,#0
	add	ix,sp
	push	af
;main.c:63: int k = num, s=0;
	ld	c,4 (ix)
	ld	b,5 (ix)
	ld	hl,#0x0000
	ex	(sp), hl
;main.c:64: cls2();
	push	bc
	call	_cls2
	pop	bc
;main.c:65: while(k--) while(data[s++] != 0);
00104$:
	ld	e, c
	ld	d, b
	dec	bc
	ld	a,d
	or	a,e
	jr	Z,00106$
	pop	de
	push	de
00101$:
	ld	l, e
	ld	h, d
	inc	de
	inc	sp
	inc	sp
	push	de
	ld	a,l
	add	a, #<(_data)
	ld	l,a
	ld	a,h
	adc	a, #>(_data)
	ld	h,a
	ld	a,(hl)
	or	a, a
	jr	Z,00104$
	jr	00101$
00106$:
;main.c:66: gotoxy(10, 6);
	ld	hl,#0x060a
	push	hl
	call	_gotoxy
;main.c:67: printf("Loading...");
	ld	hl, #___str_3
	ex	(sp),hl
	call	_printf
	pop	af
;main.c:68: gotoxy((32-strlen(data+s))/2, 8);
	ld	a,#<(_data)
	add	a, -2 (ix)
	ld	e,a
	ld	a,#>(_data)
	adc	a, -1 (ix)
	ld	d,a
	ld	c, e
	ld	b, d
	push	bc
	call	_strlen
	pop	af
	ld	a,#0x20
	sub	a, l
	ld	c,a
	ld	a,#0x00
	sbc	a, h
	ld	b,a
	srl	b
	rr	c
	ld	b,c
	push	de
	ld	a,#0x08
	push	af
	inc	sp
	push	bc
	inc	sp
	call	_gotoxy
	ld	hl, #___str_4
	ex	(sp),hl
	call	_printf
	pop	af
	pop	af
;main.c:70: pload2(num);
	ld	l,4 (ix)
	ld	h,5 (ix)
	push	hl
	call	_pload2
	ld	sp,ix
	pop	ix
	ret
___str_3:
	.ascii "Loading..."
	.db 0x00
___str_4:
	.ascii "%s"
	.db 0x00
;main.c:73: uint8 listdir(uint8 p, uint8 c, uint8 pg)
;	---------------------------------
; Function listdir
; ---------------------------------
_listdir::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-8
	add	hl,sp
	ld	sp,hl
;main.c:75: int i = pg * p;
	ld	e,4 (ix)
	ld	h,6 (ix)
	ld	l, #0x00
	ld	d, l
	ld	b, #0x08
00156$:
	add	hl,hl
	jr	NC,00157$
	add	hl,de
00157$:
	djnz	00156$
	ld	-6 (ix),l
	ld	-5 (ix),h
;main.c:78: int s = 0;
	ld	-4 (ix),#0x00
	ld	-3 (ix),#0x00
;main.c:79: while(k--) while(data[s++] != 0);
	ld	bc,#_data+0
	ld	a,-6 (ix)
	ld	-8 (ix),a
	ld	a,-5 (ix)
	ld	-7 (ix),a
00104$:
	pop	de
	push	de
	pop	hl
	push	hl
	dec	hl
	ex	(sp), hl
	ld	a,d
	or	a,e
	jr	Z,00106$
	ld	e,-4 (ix)
	ld	d,-3 (ix)
00101$:
	ld	l, e
	ld	h, d
	inc	de
	ld	-4 (ix),e
	ld	-3 (ix),d
	add	hl,bc
	ld	a,(hl)
	or	a, a
	jr	Z,00104$
	jr	00101$
00106$:
;main.c:80: attr_clear();
	push	bc
	call	_attr_clear
	pop	bc
	ld	e,#0x00
00114$:
;main.c:81: for(;j<pg;j++)
	ld	a,e
	sub	a, 6 (ix)
	jr	NC,00112$
;main.c:83: if (*(data+s) != 0)
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	add	hl,bc
	ld	a,(hl)
	or	a, a
	jr	Z,00123$
;main.c:84: printf("%03d. %-25s\n", i+j, data+s);
	push	hl
	pop	iy
	ld	l,e
	ld	h,#0x00
	ld	a,-6 (ix)
	add	a, l
	ld	l,a
	ld	a,-5 (ix)
	adc	a, h
	ld	h,a
	push	bc
	push	de
	push	iy
	push	hl
	ld	hl,#___str_5
	push	hl
	call	_printf
	ld	hl,#6
	add	hl,sp
	ld	sp,hl
	pop	de
	pop	bc
;main.c:85: while(data[s++] != 0);
00123$:
	ld	a,-4 (ix)
	ld	-2 (ix),a
	ld	a,-3 (ix)
	ld	-1 (ix),a
00109$:
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	inc	-2 (ix)
	jr	NZ,00158$
	inc	-1 (ix)
00158$:
	add	hl,bc
	ld	a,(hl)
	or	a, a
	jr	NZ,00109$
;main.c:81: for(;j<pg;j++)
	ld	a,-2 (ix)
	ld	-4 (ix),a
	ld	a,-1 (ix)
	ld	-3 (ix),a
	inc	e
	jr	00114$
00112$:
;main.c:87: attr_set(1, 0x840+c*32, 32);
	ld	l,5 (ix)
	ld	h,#0x00
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	ld	bc,#0x0840
	add	hl,bc
	ld	bc,#0x0020
	push	bc
	push	hl
	ld	a,#0x01
	push	af
	inc	sp
	call	_attr_set
	pop	af
	pop	af
	inc	sp
;main.c:88: return pg;
	ld	l,6 (ix)
	ld	sp, ix
	pop	ix
	ret
___str_5:
	.ascii "%03d. %-25s"
	.db 0x0a
	.db 0x00
	.area _CODE
	.area _INITIALIZER
	.area _CABS (ABS)
