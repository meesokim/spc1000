;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.8.0 #10562 (Linux)
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
	push	af
;main.c:20: const char *param = "SD:/\\*.tap";
;main.c:24: memset(data, 0, 0x800);
	ld	hl, #_data
	ld	(hl), #0x00
	ld	e, l
	ld	d, h
	inc	de
	ld	bc, #0x07ff
	ldir
;main.c:25: cls();
	call	_cls
;main.c:26: printf(" RPI extension box for SPC1000\n-------------------------------");
	ld	hl, #___str_1
	push	hl
	call	_printf
;main.c:28: t = pifiles(param);
	ld	hl, #___str_0
	ex	(sp),hl
	call	_pifiles
	pop	af
;main.c:29: l = t / pg;
	ld	bc, #0x000c
	push	bc
	push	hl
	call	__divsint
	pop	af
	pop	af
	ld	c, l
;main.c:30: num = pioldnum();
	push	bc
	call	_pioldnum
	pop	bc
	ld	b, l
;main.c:31: p = num / pg;
	push	bc
	ld	a, #0x0c
	push	af
	inc	sp
	push	bc
	inc	sp
	call	__divuchar
	pop	af
	pop	bc
	ld	-1 (ix), l
;main.c:32: c = num % pg;
	push	bc
	ld	a, #0x0c
	push	af
	inc	sp
	push	bc
	inc	sp
	call	__moduchar
	pop	af
	pop	bc
	ld	-2 (ix), l
;main.c:33: while(1)
00108$:
;main.c:35: gotoxy(0,2);
	push	bc
	ld	a, #0x02
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_gotoxy
	ld	h,#0x0c
	ex	(sp),hl
	inc	sp
	ld	h, -2 (ix)
	ld	l, -1 (ix)
	push	hl
	call	_listdir
	inc	sp
	ld	hl,#___str_2
	ex	(sp),hl
	call	_printf
	pop	af
	pop	bc
;main.c:38: gotoxy(4,c+2);
	ld	e, -2 (ix)
	ld	b, e
	inc	b
	inc	b
	push	bc
	push	de
	push	bc
	inc	sp
	ld	a, #0x04
	push	af
	inc	sp
	call	_gotoxy
	pop	af
	call	_getchar
	pop	de
	pop	bc
;main.c:52: c = (pg-1 > c ? c + 1: pg-1);
	ld	a, -2 (ix)
	ld	-4 (ix), a
	ld	-3 (ix), #0x00
;main.c:40: switch (ch)
	ld	a, l
	sub	a, #0x0d
	jr	Z,00105$
;main.c:43: p = (p > 0 ? p - 1: 0);
	ld	b, -1 (ix)
;main.c:40: switch (ch)
	ld	a,l
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
	ld	a, -1 (ix)
	or	a, a
	jr	Z,00112$
	ld	a, b
	dec	a
	ld	e, a
	rla
	sbc	a, a
	jr	00113$
00112$:
	ld	de, #0x0000
00113$:
	ld	-1 (ix), e
;main.c:44: break;
	jr	00108$
;main.c:45: case 0x1c:
00102$:
;main.c:46: p = (p < l ? p + 1: l);
	ld	a, -1 (ix)
	sub	a, c
	jr	NC,00114$
	inc	b
	jr	00115$
00114$:
	ld	b, c
00115$:
	ld	-1 (ix), b
;main.c:47: break;
	jp	00108$
;main.c:48: case 0x1e:
00103$:
;main.c:49: c = (c > 0 ? c - 1: 0);
	ld	a, -2 (ix)
	or	a, a
	jr	Z,00116$
	dec	e
	ld	a, e
	rla
	sbc	a, a
	jr	00117$
00116$:
	ld	de, #0x0000
00117$:
	ld	-2 (ix), e
;main.c:50: break;
	jp	00108$
;main.c:51: case 0x1f:
00104$:
;main.c:52: c = (pg-1 > c ? c + 1: pg-1);
	ld	a, -4 (ix)
	sub	a, #0x0b
	ld	a, -3 (ix)
	rla
	ccf
	rra
	sbc	a, #0x80
	jr	NC,00118$
	inc	e
	jr	00119$
00118$:
	ld	e, #0x0b
00119$:
	ld	-2 (ix), e
;main.c:53: break;
	jp	00108$
;main.c:54: case 0x0d:
00105$:
;main.c:55: run((int)p * pg + c);
	ld	e, -1 (ix)
	ld	d, #0x00
	ld	l, e
	ld	h, d
	add	hl, hl
	add	hl, de
	add	hl, hl
	add	hl, hl
	pop	de
	push	de
	add	hl, de
	push	bc
	push	hl
	call	_run
	pop	af
	pop	bc
;main.c:57: }
;main.c:59: }
	jp	00108$
___str_0:
	.ascii "SD:/"
	.db 0x5c
	.ascii "*.tap"
	.db 0x00
___str_1:
	.ascii " RPI extension box for SPC1000"
	.db 0x0a
	.ascii "-------------------------------"
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
	ld	c, 4 (ix)
	ld	b, 5 (ix)
	ld	hl, #0x0000
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
	ld	a, d
	or	a, e
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
	push	de
	ld	de, #_data
	add	hl, de
	pop	de
	ld	a, (hl)
	or	a, a
	jr	Z,00104$
	jr	00101$
00106$:
;main.c:66: gotoxy(10, 6);
	ld	de, #0x060a
	push	de
	call	_gotoxy
;main.c:67: printf("Loading...");
	ld	hl, #___str_3
	ex	(sp),hl
	call	_printf
	pop	af
;main.c:68: gotoxy((32-strlen(data+s))/2, 8);
	ld	a, #<(_data)
	add	a, -2 (ix)
	ld	e, a
	ld	a, #>(_data)
	adc	a, -1 (ix)
	ld	d, a
	ld	c, e
	ld	b, d
	push	bc
	call	_strlen
	pop	af
	ld	c, l
	ld	b, h
	ld	hl, #0x0020
	cp	a, a
	sbc	hl, bc
	srl	h
	rr	l
	ld	b, l
	push	de
	ld	a, #0x08
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
	ld	l, 4 (ix)
	ld	h, 5 (ix)
	push	hl
	call	_pload2
;main.c:71: }
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
	ld	hl, #-6
	add	hl, sp
	ld	sp, hl
;main.c:75: int i = pg * p;
	ld	e, 4 (ix)
	ld	h, 6 (ix)
	ld	l, #0x00
	ld	d, l
	ld	b, #0x08
00165$:
	add	hl, hl
	jr	NC,00166$
	add	hl, de
00166$:
	djnz	00165$
	ld	c, l
	ld	b, h
;main.c:78: int s = 0;
	ld	hl, #0x0000
	ex	(sp), hl
;main.c:79: while(k--) while(data[s++] != 0);
	ld	-4 (ix), c
	ld	-3 (ix), b
00104$:
	ld	e, -4 (ix)
	ld	d, -3 (ix)
	ld	l, -4 (ix)
	ld	h, -3 (ix)
	dec	hl
	ld	-4 (ix), l
	ld	-3 (ix), h
	ld	a, d
	or	a, e
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
	ld	a, l
	add	a, #<(_data)
	ld	l, a
	ld	a, h
	adc	a, #>(_data)
	ld	h, a
	ld	a, (hl)
	or	a, a
	jr	Z,00104$
	jr	00101$
00106$:
;main.c:80: attr_clear();
	push	bc
	call	_attr_clear
	pop	bc
	ld	e, #0x00
00114$:
;main.c:81: for(;j<pg;j++)
	ld	a, e
	sub	a, 6 (ix)
	jr	NC,00112$
;main.c:83: if (*(data+s) != 0)
	ld	a, #<(_data)
	add	a, -6 (ix)
	ld	l, a
	ld	a, #>(_data)
	adc	a, -5 (ix)
	ld	h, a
	ld	a, (hl)
	or	a, a
	jr	Z,00123$
;main.c:84: printf("%03d. %-25s\n", i+j, data+s);
	push	hl
	pop	iy
	ld	l, e
	ld	h, #0x00
	add	hl, bc
	push	bc
	push	de
	push	iy
	push	hl
	ld	hl, #___str_5
	push	hl
	call	_printf
	ld	hl, #6
	add	hl, sp
	ld	sp, hl
	pop	de
	pop	bc
;main.c:85: while(data[s++] != 0);
00123$:
	ld	a, -6 (ix)
	ld	-2 (ix), a
	ld	a, -5 (ix)
	ld	-1 (ix), a
00109$:
	ld	d, -2 (ix)
	ld	h, -1 (ix)
	inc	-2 (ix)
	jr	NZ,00167$
	inc	-1 (ix)
00167$:
	ld	a, d
	add	a, #<(_data)
	ld	l, a
	ld	a, h
	adc	a, #>(_data)
	ld	h, a
	ld	a, (hl)
	or	a, a
	jr	NZ,00109$
;main.c:81: for(;j<pg;j++)
	ld	a, -2 (ix)
	ld	-6 (ix), a
	ld	a, -1 (ix)
	ld	-5 (ix), a
	inc	e
	jr	00114$
00112$:
;main.c:87: attr_set(1, 0x840+c*32, 32);
	ld	l, 5 (ix)
	ld	h, #0x00
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	ld	bc, #0x0840
	add	hl, bc
	ld	bc, #0x0020
	push	bc
	push	hl
	ld	a, #0x01
	push	af
	inc	sp
	call	_attr_set
	pop	af
	pop	af
	inc	sp
;main.c:88: return pg;
	ld	l, 6 (ix)
;main.c:89: }
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
