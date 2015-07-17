;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.4.0 #8981 (Apr  5 2014) (MINGW64)
; This file was generated Fri Jul 17 15:34:33 2015
;--------------------------------------------------------
	.module main
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _dir
	.globl _main
	.globl _cas_save
	.globl _cas_load
	.globl _sd_format
	.globl _sd_read
	.globl _sd_write
	.globl _sd_init
	.globl _getchar
	.globl _printf
	.globl _data
	.globl _dsk2dsk
	.globl _cas2dsk
	.globl _dsk2cas
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
_ioport	=	0x0000
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_data::
	.ds 4097
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
;main.c:10: void main(void)
;	---------------------------------
; Function main
; ---------------------------------
_main_start::
_main:
;main.c:13: printf("FDD check\n");
	ld	hl,#___str_0
	push	hl
	call	_printf
	pop	af
;main.c:14: sd_init();
	call	_sd_init
;main.c:15: printf("FDD initialized\n");
	ld	hl,#___str_1
	push	hl
	call	_printf
	pop	af
;main.c:16: while(1)
00109$:
;main.c:19: printf("Disk Utility v0.1\n0. dsk2dsk\n1. cas2dsk\n2. dsk2cas\n3. format disk A\n4. format disk B\n?");
	ld	hl,#___str_2
	push	hl
	call	_printf
	pop	af
;main.c:20: c = getchar();
	call	_getchar
;main.c:21: printf("%c\n", c);
	ld	c,l
	ld	b,#0x00
	ld	de,#___str_3
	push	hl
	push	bc
	push	de
	call	_printf
	pop	af
	pop	af
	pop	hl
;main.c:22: switch (c)
	ld	a,l
	sub	a, #0x30
	jr	C,00109$
	ld	a,#0x35
	sub	a, l
	jr	C,00109$
	ld	a,l
	add	a,#0xD0
	ld	e,a
	ld	d,#0x00
	ld	hl,#00125$
	add	hl,de
	add	hl,de
;main.c:24: case '0':
	jp	(hl)
00125$:
	jr	00101$
	jr	00102$
	jr	00103$
	jr	00104$
	jr	00105$
	jr	00106$
00101$:
;main.c:25: dsk2dsk();
	call	_dsk2dsk
;main.c:26: break;
	jr	00109$
;main.c:27: case '1':
00102$:
;main.c:28: cas2dsk();
	call	_cas2dsk
;main.c:29: break;
	jr	00109$
;main.c:30: case '2':
00103$:
;main.c:31: dsk2cas();
	call	_dsk2cas
;main.c:32: break;
	jr	00109$
;main.c:33: case '3':
00104$:
;main.c:34: sd_format(0);
	xor	a, a
	push	af
	inc	sp
	call	_sd_format
	inc	sp
;main.c:35: break;
	jr	00109$
;main.c:36: case '4':
00105$:
;main.c:37: sd_format(1);
	ld	a,#0x01
	push	af
	inc	sp
	call	_sd_format
	inc	sp
;main.c:38: break;
	jr	00109$
;main.c:39: case '5':
00106$:
;main.c:40: return;
;main.c:41: }
	ret
_main_end::
___str_0:
	.ascii "FDD check"
	.db 0x0A
	.db 0x00
___str_1:
	.ascii "FDD initialized"
	.db 0x0A
	.db 0x00
___str_2:
	.ascii "Disk Utility v0.1"
	.db 0x0A
	.ascii "0. dsk2dsk"
	.db 0x0A
	.ascii "1. cas2dsk"
	.db 0x0A
	.ascii "2. dsk2cas"
	.db 0x0A
	.ascii "3. format"
	.ascii " disk A"
	.db 0x0A
	.ascii "4. format disk B"
	.db 0x0A
	.ascii "?"
	.db 0x00
___str_3:
	.ascii "%c"
	.db 0x0A
	.db 0x00
;main.c:44: void dsk2dsk()
;	---------------------------------
; Function dsk2dsk
; ---------------------------------
_dsk2dsk_start::
_dsk2dsk:
;main.c:47: for(t = 0; t < 80; t++)
	ld	e,#0x00
00102$:
;main.c:49: printf("FDD reading..track:%d\n", t);
	ld	c,e
	ld	b,#0x00
	ld	hl,#___str_4
	push	bc
	push	de
	push	bc
	push	hl
	call	_printf
	pop	af
	pop	af
	pop	de
	pop	bc
;main.c:50: sd_read(16,0,t,1,data);
	ld	hl,#_data
	push	bc
	push	de
	push	hl
	ld	d,#0x01
	push	de
	ld	hl,#0x0010
	push	hl
	call	_sd_read
	ld	hl,#6
	add	hl,sp
	ld	sp,hl
	pop	de
	pop	bc
;main.c:51: printf("FDD writing..track:%d\n", t);
	ld	hl,#___str_5
	push	de
	push	bc
	push	hl
	call	_printf
	pop	af
	pop	af
	pop	de
;main.c:52: sd_write(16,1,t,1,data);
	ld	hl,#_data
	push	de
	push	hl
	ld	d,#0x01
	push	de
	ld	hl,#0x0110
	push	hl
	call	_sd_write
	ld	hl,#6
	add	hl,sp
	ld	sp,hl
	pop	de
;main.c:47: for(t = 0; t < 80; t++)
	inc	e
	ld	a,e
	sub	a, #0x50
	jr	C,00102$
	ret
_dsk2dsk_end::
___str_4:
	.ascii "FDD reading..track:%d"
	.db 0x0A
	.db 0x00
___str_5:
	.ascii "FDD writing..track:%d"
	.db 0x0A
	.db 0x00
;main.c:55: void cas2dsk()
;	---------------------------------
; Function cas2dsk
; ---------------------------------
_cas2dsk_start::
_cas2dsk:
;main.c:58: for(t = 0; t < 80; t++)
00102$:
;main.c:60: printf("CAS loading..\n");
	ld	hl,#___str_6
	push	hl
	call	_printf
	pop	af
;main.c:61: cas_load(data, 0x1001);
	ld	hl,#_data
	ld	bc,#0x1001
	push	bc
	push	hl
	call	_cas_load
	pop	af
	pop	af
;main.c:62: t = data[0x1000];
	ld	hl, #_data + 4096
	ld	d,(hl)
;main.c:63: printf("FDD writing..track:%d\n", t);
	ld	c,d
	ld	b,#0x00
	ld	hl,#___str_7
	push	de
	push	bc
	push	hl
	call	_printf
	pop	af
	pop	af
	pop	de
;main.c:64: sd_write(16,0,t,1,data);
	ld	hl,#_data
	push	de
	push	hl
	ld	a,#0x01
	push	af
	inc	sp
	push	de
	inc	sp
	ld	hl,#0x0010
	push	hl
	call	_sd_write
	ld	hl,#6
	add	hl,sp
	ld	sp,hl
	pop	de
;main.c:58: for(t = 0; t < 80; t++)
	inc	d
	ld	a,d
	sub	a, #0x50
	jr	C,00102$
;main.c:66: printf("FDD transfer completed\n");
	ld	hl,#___str_8
	push	hl
	call	_printf
	pop	af
	ret
_cas2dsk_end::
___str_6:
	.ascii "CAS loading.."
	.db 0x0A
	.db 0x00
___str_7:
	.ascii "FDD writing..track:%d"
	.db 0x0A
	.db 0x00
___str_8:
	.ascii "FDD transfer completed"
	.db 0x0A
	.db 0x00
;main.c:69: void dsk2cas()
;	---------------------------------
; Function dsk2cas
; ---------------------------------
_dsk2cas_start::
_dsk2cas:
;main.c:72: for(t = 0; t < 80; t++)
	ld	e,#0x00
00102$:
;main.c:74: printf("FDD reading..track:%d\n", t);
	ld	c,e
	ld	b,#0x00
	ld	hl,#___str_9
	push	bc
	push	de
	push	bc
	push	hl
	call	_printf
	pop	af
	pop	af
	pop	de
	pop	bc
;main.c:75: sd_read(16,0,t,1,data);
	ld	hl,#_data
	push	bc
	push	de
	push	hl
	ld	d,#0x01
	push	de
	ld	hl,#0x0010
	push	hl
	call	_sd_read
	ld	hl,#6
	add	hl,sp
	ld	sp,hl
	pop	de
	pop	bc
;main.c:76: printf("CAS recording..track:%d\n", t);
	ld	hl,#___str_10
	push	de
	push	bc
	push	hl
	call	_printf
	pop	af
	pop	af
	pop	de
;main.c:77: data[0x1000] = t;
	ld	hl,#_data + 4096
	ld	(hl),e
;main.c:78: cas_save(data, 0x1001);
	ld	hl,#_data
	push	de
	ld	bc,#0x1001
	push	bc
	push	hl
	call	_cas_save
	pop	af
	pop	af
	pop	de
;main.c:72: for(t = 0; t < 80; t++)
	inc	e
	ld	a,e
	sub	a, #0x50
	jr	C,00102$
;main.c:80: printf("CAS recording completed\n");
	ld	hl,#___str_11
	push	hl
	call	_printf
	pop	af
	ret
_dsk2cas_end::
___str_9:
	.ascii "FDD reading..track:%d"
	.db 0x0A
	.db 0x00
___str_10:
	.ascii "CAS recording..track:%d"
	.db 0x0A
	.db 0x00
___str_11:
	.ascii "CAS recording completed"
	.db 0x0A
	.db 0x00
;main.c:83: void dir(char *pp)
;	---------------------------------
; Function dir
; ---------------------------------
_dir_start::
_dir:
	push	ix
	ld	ix,#0
	add	ix,sp
;main.c:86: char *p = pp;
	ld	c,4 (ix)
	ld	b,5 (ix)
;main.c:88: while(*p != 0 || p < pp + 256)
	ld	hl,#0x0100
	add	hl,bc
	ex	de,hl
00102$:
	ld	a,(bc)
	or	a, a
	jr	NZ,00103$
	ld	a,c
	sub	a, e
	ld	a,b
	sbc	a, d
	jr	NC,00105$
00103$:
;main.c:90: printf("%d.%s (%d)\n", i, p, *(p+15)*256);
	ld	l, c
	ld	h, b
	push	bc
	ld	bc, #0x000F
	add	hl, bc
	pop	bc
	ld	a,(hl)
	ld	l,a
	rla
	sbc	a, a
	ld	h,l
	ld	l,#0x00
	push	bc
	push	de
	push	hl
	push	bc
	ld	hl,#0x0001
	push	hl
	ld	hl,#___str_12
	push	hl
	call	_printf
	ld	hl,#8
	add	hl,sp
	ld	sp,hl
	pop	de
	pop	bc
;main.c:91: p += 1;
	inc	bc
	jr	00102$
00105$:
	pop	ix
	ret
_dir_end::
___str_12:
	.ascii "%d.%s (%d)"
	.db 0x0A
	.db 0x00
	.area _CODE
	.area _INITIALIZER
	.area _CABS (ABS)
