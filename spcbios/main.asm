;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 4.0.0 #11528 (Linux)
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
	.ds 2048
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
;main.c:17: void main(void)
;	---------------------------------
; Function main
; ---------------------------------
_main::
	push	ix
	ld	ix,#0
	add	ix,sp
	push	af
	push	af
	dec	sp
;main.c:24: *addr = 0;
	ld	hl, #0x0c7f
	ld	(hl), #0x00
;main.c:29: memset(data, 0, sizeof(data));
	ld	hl, #_data
	ld	(hl), #0x00
	ld	e, l
	ld	d, h
	inc	de
	ld	bc, #0x07ff
	ldir
;main.c:30: cls();
	call	_cls
;main.c:31: printf(" RPI extension box for SPC1000\n-----------------------------");
	ld	hl, #___str_0
	push	hl
	call	_printf
;main.c:33: t = pifiles("*.tap");
	ld	hl, #___str_1
	ex	(sp),hl
	call	_pifiles
	pop	af
	ld	c, l
	ld	b, h
;main.c:34: l = (t / PAGE);
	push	bc
	ld	hl, #0x000c
	push	hl
	push	bc
	call	__divsint
	pop	af
	pop	af
;main.c:35: num = pioldnum();
	ld	-5 (ix), l
	call	_pioldnum
	pop	bc
;main.c:37: p = num / PAGE;
	xor	a, a
	ld	-2 (ix), a
;main.c:38: c = num - PAGE * p;
	xor	a, a
	ld	e, a
	add	a, a
	add	a, e
	add	a, a
	add	a, a
	neg
	ld	-1 (ix), a
;main.c:39: while(1)
00122$:
;main.c:41: gotoxy(0,2);
	push	bc
	ld	a, #0x02
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_gotoxy
	pop	af
	pop	bc
;main.c:42: if (p == l)
	ld	a, -5 (ix)
	sub	a, -2 (ix)
	jr	NZ,00102$
;main.c:43: pg = t % PAGE - 1;
	push	bc
	ld	hl, #0x000c
	push	hl
	push	bc
	call	__modsint
	pop	af
	pop	af
	pop	bc
	dec	l
	jr	00103$
00102$:
;main.c:45: pg = PAGE - 1;
	ld	l, #0x0b
00103$:
;main.c:46: if (c > pg - 1)
	ld	e, l
	ld	d, #0x00
	ld	a, e
	add	a, #0xff
	ld	-4 (ix), a
	ld	a, d
	adc	a, #0xff
	ld	-3 (ix), a
	ld	e, -1 (ix)
	ld	d, #0x00
	ld	a, -4 (ix)
	sub	a, e
	ld	a, -3 (ix)
	sbc	a, d
	jp	PO, 00188$
	xor	a, #0x80
00188$:
	jp	P, 00105$
;main.c:47: c = pg - 1;
	ld	a, l
	add	a, #0xff
	ld	-1 (ix), a
00105$:
;main.c:48: listdir(p, c);
	push	bc
	ld	h, -1 (ix)
	ld	l, -2 (ix)
	push	hl
	call	_listdir
	ld	hl, #___str_2
	ex	(sp),hl
	call	_printf
	pop	af
	pop	bc
;main.c:51: gotoxy(4,c+2);
	ld	a, -1 (ix)
	add	a, #0x02
	push	bc
	ld	d,a
	ld	e,#0x04
	push	de
	call	_gotoxy
	pop	af
	call	_getchar
	pop	bc
;main.c:46: if (c > pg - 1)
	ld	e, -1 (ix)
	ld	d, #0x00
;main.c:53: switch (ch)
	ld	a,l
	cp	a,#0x0d
	jr	Z,00118$
	cp	a,#0x1c
	jr	Z,00109$
	cp	a,#0x1d
	jr	Z,00106$
	cp	a,#0x1e
	jr	Z,00112$
	sub	a, #0x1f
	jr	Z,00115$
	jp	00122$
;main.c:55: case 0x1d:
00106$:
;main.c:56: if (p > 0)
	ld	a, -2 (ix)
	or	a, a
	jp	Z, 00122$
;main.c:57: p--;
	dec	-2 (ix)
;main.c:58: break;
	jp	00122$
;main.c:59: case 0x1c:
00109$:
;main.c:60: if (p < l)
	ld	a, -2 (ix)
	sub	a, -5 (ix)
	jp	NC, 00122$
;main.c:61: p++;
	inc	-2 (ix)
;main.c:62: break;
	jp	00122$
;main.c:63: case 0x1e:
00112$:
;main.c:64: if (c > 0)
	ld	a, -1 (ix)
	or	a, a
	jp	Z, 00122$
;main.c:65: c--;
	dec	-1 (ix)
;main.c:66: break;
	jp	00122$
;main.c:67: case 0x1f:
00115$:
;main.c:68: if (c < pg-1)
	ld	a, e
	sub	a, -4 (ix)
	ld	a, d
	sbc	a, -3 (ix)
	jp	PO, 00194$
	xor	a, #0x80
00194$:
	jp	P, 00122$
;main.c:69: c++;
	inc	-1 (ix)
;main.c:70: break;
	jp	00122$
;main.c:71: case 0x0d:
00118$:
;main.c:72: run((int)p * PAGE + c);
	ld	l, -2 (ix)
	ld	h, #0x00
	push	de
	ld	e, l
	ld	d, h
	add	hl, hl
	add	hl, de
	add	hl, hl
	add	hl, hl
	pop	de
	add	hl, de
	push	bc
	push	hl
	call	_run
	pop	af
	pop	bc
;main.c:73: break;
;main.c:76: }
;main.c:78: }
	jp	00122$
___str_0:
	.ascii " RPI extension box for SPC1000"
	.db 0x0a
	.ascii "-----------------------------"
	.db 0x00
___str_1:
	.ascii "*.tap"
	.db 0x00
___str_2:
	.ascii "-------------------------------       RETURN for Execution"
	.db 0x00
;main.c:80: void run(int num)
;	---------------------------------
; Function run
; ---------------------------------
_run::
	push	ix
	ld	ix,#0
	add	ix,sp
	push	af
;main.c:82: int k = num, s=0;
	ld	c, 4 (ix)
	ld	b, 5 (ix)
	ld	hl, #0x0000
	ex	(sp), hl
;main.c:83: cls2();
	push	bc
	call	_cls2
	pop	bc
;main.c:84: while(k--) while(data[s++] != 0);
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
;main.c:85: gotoxy(10, 6);
	ld	de, #0x060a
	push	de
	call	_gotoxy
;main.c:86: printf("Loading...");
	ld	hl, #___str_3
	ex	(sp),hl
	call	_printf
	pop	af
;main.c:87: gotoxy((32-strlen(data+s))/2, 8);
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
	ld	a, #0x20
	sub	a, l
	ld	c, a
	ld	a, #0x00
	sbc	a, h
	ld	b, a
	srl	b
	rr	c
	push	de
	ld	b, #0x08
	push	bc
	call	_gotoxy
	ld	hl, #___str_4
	ex	(sp),hl
	call	_printf
	pop	af
	pop	af
;main.c:89: pload2(num);
	ld	l, 4 (ix)
	ld	h, 5 (ix)
	push	hl
	call	_pload2
;main.c:90: }
	ld	sp,ix
	pop	ix
	ret
___str_3:
	.ascii "Loading..."
	.db 0x00
___str_4:
	.ascii "%s"
	.db 0x00
;main.c:92: void listdir(uint8 p, uint8 c)
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
;main.c:94: int i = PAGE * p;
	ld	c, 4 (ix)
	ld	b, #0x00
	ld	l, c
	ld	h, b
	add	hl, hl
	add	hl, bc
	add	hl, hl
	add	hl, hl
	ld	c, l
	ld	b, h
;main.c:97: int s = 0;
	xor	a, a
	ld	-2 (ix), a
	ld	-1 (ix), a
;main.c:98: while(k--) while(data[s++] != 0);
	ld	de, #_data+0
	inc	sp
	inc	sp
	push	bc
00104$:
	pop	hl
	push	hl
	ld	a, -6 (ix)
	add	a, #0xff
	ld	-6 (ix), a
	ld	a, -5 (ix)
	adc	a, #0xff
	ld	-5 (ix), a
	ld	a, h
	or	a, l
	jr	Z,00106$
	ld	a, -2 (ix)
	ld	-4 (ix), a
	ld	a, -1 (ix)
	ld	-3 (ix), a
00101$:
	ld	l, -4 (ix)
	ld	h, -3 (ix)
	inc	-4 (ix)
	jr	NZ,00166$
	inc	-3 (ix)
00166$:
	ld	a, -4 (ix)
	ld	-2 (ix), a
	ld	a, -3 (ix)
	ld	-1 (ix), a
	add	hl, de
	ld	a, (hl)
	or	a, a
	jr	Z,00104$
	jr	00101$
00106$:
;main.c:99: attr_clear();
	push	bc
	push	de
	call	_attr_clear
	pop	de
	pop	bc
	xor	a, a
	ld	-3 (ix), a
00115$:
;main.c:100: for(;j<PAGE;j++)
	ld	a, -3 (ix)
	sub	a, #0x0c
	jr	NC,00113$
;main.c:102: if (*(data+s) != 0)
	ld	a, #<(_data)
	add	a, -2 (ix)
	ld	-5 (ix), a
	ld	a, #>(_data)
	adc	a, -1 (ix)
	ld	-4 (ix), a
	ld	l, -5 (ix)
	ld	h, -4 (ix)
	ld	a, (hl)
	or	a, a
	jr	Z,00108$
;main.c:103: printf("%03d. %-25s\n", i+j, data+s);
	ld	e, -5 (ix)
	ld	d, -4 (ix)
	ld	l, -3 (ix)
	ld	h, #0x00
	add	hl, bc
	push	bc
	push	de
	push	hl
	ld	hl, #___str_5
	push	hl
	call	_printf
	ld	hl, #6
	add	hl, sp
	ld	sp, hl
	pop	bc
	jr	00124$
00108$:
;main.c:105: printf("%-30s\n", " ");
	push	bc
	ld	hl, #___str_7
	push	hl
	ld	hl, #___str_6
	push	hl
	call	_printf
	pop	af
	pop	af
	pop	bc
;main.c:106: while(data[s++] != 0);
00124$:
00110$:
	ld	a, #<(_data)
	add	a, -2 (ix)
	ld	e, a
	ld	a, #>(_data)
	adc	a, -1 (ix)
	ld	d, a
	inc	-2 (ix)
	jr	NZ,00167$
	inc	-1 (ix)
00167$:
	ld	a, (de)
	or	a, a
	jr	NZ,00110$
;main.c:100: for(;j<PAGE;j++)
	inc	-3 (ix)
	jr	00115$
00113$:
;main.c:108: attr_set(1, 0x840+c*32, 32);
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
;main.c:109: }
	ld	sp,ix
	pop	ix
	ret
___str_5:
	.ascii "%03d. %-25s"
	.db 0x0a
	.db 0x00
___str_6:
	.ascii "%-30s"
	.db 0x0a
	.db 0x00
___str_7:
	.ascii " "
	.db 0x00
	.area _CODE
	.area _INITIALIZER
	.area _CABS (ABS)
