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
	dec	sp
;main.c:22: *addr = 0;
	ld	hl, #0x0c7f
	ld	(hl), #0x00
;main.c:27: memset(data, 0, sizeof(data));
	ld	hl, #_data
	ld	(hl), #0x00
	ld	e, l
	ld	d, h
	inc	de
	ld	bc, #0x07ff
	ldir
;main.c:28: cls();
	call	_cls
;main.c:29: printf(" RPI extension box for SPC1000\n-------------------------------");
	ld	hl, #___str_0
	push	hl
	call	_printf
;main.c:31: t = pifiles("*.tap");
	ld	hl, #___str_1
	ex	(sp),hl
	call	_pifiles
	pop	af
	ld	c, l
	ld	b, h
;main.c:32: l = (t / PAGE);
	push	bc
	ld	hl, #0x000c
	push	hl
	push	bc
	call	__divsint
	pop	af
	pop	af
;main.c:33: num = pioldnum();
	ld	-2 (ix), l
	call	_pioldnum
	pop	bc
;main.c:35: p = num / PAGE;
	ld	-1 (ix), #0x00
;main.c:36: c = num - PAGE * p;
	xor	a, a
	ld	e, a
	add	a, a
	add	a, e
	add	a, a
	add	a, a
	neg
	ld	-3 (ix), a
;main.c:37: while(1)
00122$:
;main.c:39: gotoxy(0,2);
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
;main.c:40: if (p == l) {
	ld	a, -2 (ix)
	sub	a, -1 (ix)
	jr	NZ,00104$
;main.c:41: pg = t % PAGE - 1;
	push	bc
	ld	hl, #0x000c
	push	hl
	push	bc
	call	__modsint
	pop	af
	pop	af
	ld	e, l
	pop	bc
	dec	e
;main.c:42: if (c > pg)
	ld	a, e
	sub	a, -3 (ix)
	jr	NC,00105$
;main.c:43: c = pg;
	ld	-3 (ix), e
	jr	00105$
00104$:
;main.c:46: pg = PAGE - 1;
	ld	e, #0x0b
00105$:
;main.c:47: listdir(p, c);
	push	bc
	push	de
	ld	h, -3 (ix)
	ld	l, -1 (ix)
	push	hl
	call	_listdir
	ld	hl, #___str_2
	ex	(sp),hl
	call	_printf
	pop	af
	pop	de
	pop	bc
;main.c:50: gotoxy(4,c+2);
	ld	d, -3 (ix)
	inc	d
	inc	d
	push	bc
	push	de
	push	de
	inc	sp
	ld	a, #0x04
	push	af
	inc	sp
	call	_gotoxy
	pop	af
	call	_getchar
	pop	de
	pop	bc
	ld	a, l
;main.c:52: switch (ch)
	cp	a, #0x0d
	jr	Z,00118$
	cp	a, #0x1c
	jr	Z,00109$
	cp	a, #0x1d
	jr	Z,00106$
	cp	a, #0x1e
	jr	Z,00112$
	sub	a, #0x1f
	jr	Z,00115$
	jr	00122$
;main.c:54: case 0x1d:
00106$:
;main.c:55: if (p > 0)
	ld	a, -1 (ix)
	or	a, a
	jr	Z,00122$
;main.c:56: p--;
	dec	-1 (ix)
;main.c:57: break;
	jp	00122$
;main.c:58: case 0x1c:
00109$:
;main.c:59: if (p < l)
	ld	a, -1 (ix)
	sub	a, -2 (ix)
	jp	NC, 00122$
;main.c:60: p++;
	inc	-1 (ix)
;main.c:61: break;
	jp	00122$
;main.c:62: case 0x1e:
00112$:
;main.c:63: if (c > 0)
	ld	a, -3 (ix)
	or	a, a
	jp	Z, 00122$
;main.c:64: c--;
	dec	-3 (ix)
;main.c:65: break;
	jp	00122$
;main.c:66: case 0x1f:
00115$:
;main.c:67: if (c < pg)
	ld	a, -3 (ix)
	sub	a, e
	jp	NC, 00122$
;main.c:68: c++;
	inc	-3 (ix)
;main.c:69: break;
	jp	00122$
;main.c:70: case 0x0d:
00118$:
;main.c:71: run((int)p * PAGE + c);
	ld	e, -1 (ix)
	ld	d, #0x00
	ld	l, e
	ld	h, d
	add	hl, hl
	add	hl, de
	add	hl, hl
	add	hl, hl
	ld	e, -3 (ix)
	ld	d, #0x00
	add	hl, de
	push	bc
	push	hl
	call	_run
	pop	af
	pop	bc
;main.c:72: break;
;main.c:75: }
;main.c:77: }
	jp	00122$
___str_0:
	.ascii " RPI extension box for SPC1000"
	.db 0x0a
	.ascii "-------------------------------"
	.db 0x00
___str_1:
	.ascii "*.tap"
	.db 0x00
___str_2:
	.ascii "-------------------------------       meeso.kim@gmail.com"
	.db 0x00
;main.c:79: void run(int num)
;	---------------------------------
; Function run
; ---------------------------------
_run::
	push	ix
	ld	ix,#0
	add	ix,sp
	push	af
;main.c:81: int k = num, s=0;
	ld	c, 4 (ix)
	ld	b, 5 (ix)
	ld	hl, #0x0000
	ex	(sp), hl
;main.c:82: cls2();
	push	bc
	call	_cls2
	pop	bc
;main.c:83: while(k--) while(data[s++] != 0);
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
;main.c:84: gotoxy(10, 6);
	ld	de, #0x060a
	push	de
	call	_gotoxy
;main.c:85: printf("Loading...");
	ld	hl, #___str_3
	ex	(sp),hl
	call	_printf
	pop	af
;main.c:86: gotoxy((32-strlen(data+s))/2, 8);
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
;main.c:88: pload2(num);
	ld	l, 4 (ix)
	ld	h, 5 (ix)
	push	hl
	call	_pload2
;main.c:89: }
	ld	sp,ix
	pop	ix
	ret
___str_3:
	.ascii "Loading..."
	.db 0x00
___str_4:
	.ascii "%s"
	.db 0x00
;main.c:91: void listdir(uint8 p, uint8 c)
;	---------------------------------
; Function listdir
; ---------------------------------
_listdir::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl, #-7
	add	hl, sp
	ld	sp, hl
;main.c:93: int i = PAGE * p;
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
;main.c:96: int s = 0;
	ld	-2 (ix), #0x00
	ld	-1 (ix), #0x00
;main.c:97: while(k--) while(data[s++] != 0);
	ld	-5 (ix), c
	ld	-4 (ix), b
00104$:
	ld	e, -5 (ix)
	ld	d, -4 (ix)
	ld	l, -5 (ix)
	ld	h, -4 (ix)
	dec	hl
	ld	-5 (ix), l
	ld	-4 (ix), h
	ld	a, d
	or	a, e
	jr	Z,00106$
	ld	e, -2 (ix)
	ld	d, -1 (ix)
00101$:
	ld	l, e
	ld	h, d
	inc	de
	ld	-2 (ix), e
	ld	-1 (ix), d
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
;main.c:98: attr_clear();
	push	bc
	call	_attr_clear
	pop	bc
	ld	-3 (ix), #0x00
00115$:
;main.c:99: for(;j<PAGE;j++)
	ld	a, -3 (ix)
	sub	a, #0x0c
	jr	NC,00113$
;main.c:101: if (*(data+s) != 0)
	ld	a, #<(_data)
	add	a, -2 (ix)
	ld	-7 (ix), a
	ld	a, #>(_data)
	adc	a, -1 (ix)
	ld	-6 (ix), a
	pop	hl
	push	hl
	ld	a, (hl)
	or	a, a
	jr	Z,00108$
;main.c:102: printf("%03d. %-26s\n", i+j, data+s);
	pop	de
	push	de
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
;main.c:104: printf("%-31s\n", " ");
	push	bc
	ld	hl, #___str_7
	push	hl
	ld	hl, #___str_6
	push	hl
	call	_printf
	pop	af
	pop	af
	pop	bc
;main.c:105: while(data[s++] != 0);
00124$:
	ld	a, -2 (ix)
	ld	-7 (ix), a
	ld	a, -1 (ix)
	ld	-6 (ix), a
00110$:
	pop	de
	push	de
	inc	-7 (ix)
	jr	NZ,00166$
	inc	-6 (ix)
00166$:
	ld	hl, #_data
	add	hl, de
	ld	a, (hl)
	or	a, a
	jr	NZ,00110$
;main.c:99: for(;j<PAGE;j++)
	ld	a, -7 (ix)
	ld	-2 (ix), a
	ld	a, -6 (ix)
	ld	-1 (ix), a
	inc	-3 (ix)
	jr	00115$
00113$:
;main.c:107: attr_set(1, 0x840+c*32, 32);
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
;main.c:108: }
	ld	sp,ix
	pop	ix
	ret
___str_5:
	.ascii "%03d. %-26s"
	.db 0x0a
	.db 0x00
___str_6:
	.ascii "%-31s"
	.db 0x0a
	.db 0x00
___str_7:
	.ascii " "
	.db 0x00
	.area _CODE
	.area _INITIALIZER
	.area _CABS (ABS)
