;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.4.0 #8981 (Apr  5 2014) (MINGW64)
; This file was generated Fri Jul 17 15:34:33 2015
;--------------------------------------------------------
	.module iocs
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _cls
	.globl _FInfo
	.globl _putchar
	.globl _getchar
	.globl _cas_load
	.globl _cas_save
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
_FInfo::
	.ds 18
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
;iocs.c:6: void putchar(  char c) 
;	---------------------------------
; Function putchar
; ---------------------------------
_putchar_start::
_putchar:
;iocs.c:18: __endasm;
	ld hl,#2
	add hl,sp
	ld a, (hl)
	cp #0x0a
	jr nz, next
	ld a, #0x0d
	next:
	call 0x0864
	ret
_putchar_end::
;iocs.c:21: char getchar()
;	---------------------------------
; Function getchar
; ---------------------------------
_getchar_start::
_getchar:
;iocs.c:26: __endasm;
	call 0x0c62
	ld (_io_val), a
;iocs.c:27: return io_val;
	ld	iy,#_io_val
	ld	l,0 (iy)
	ret
_getchar_end::
;iocs.c:30: char cas_load(unsigned char *data, int len)
;	---------------------------------
; Function cas_load
; ---------------------------------
_cas_load_start::
_cas_load:
;iocs.c:47: __endasm;
	push ix
	ld ix,#4
	add ix,sp
	ld l,(ix)
	ld h,1(ix)
	ld (0x13aa), hl
	ld l,2(ix)
	ld h,3(ix)
	ld (0x13a8), hl
	call 0x0134
	ld (_io_val), a
	pop ix
;iocs.c:48: return io_val;
	ld	iy,#_io_val
	ld	l,0 (iy)
	ret
_cas_load_end::
;iocs.c:51: void cas_save(unsigned char *data, int len)
;	---------------------------------
; Function cas_save
; ---------------------------------
_cas_save_start::
_cas_save:
;iocs.c:67: __endasm;
	push ix
	ld ix,#4
	add ix,sp
	ld l,(ix)
	ld h,1(ix)
	ld (0x13aa), hl
	ld l,2(ix)
	ld h,3(ix)
	ld (0x13a8), hl
	call 0x00b6
	pop ix
;iocs.c:68: return;	
	ret
_cas_save_end::
;iocs.c:71: void cls()
;	---------------------------------
; Function cls
; ---------------------------------
_cls_start::
_cls:
;iocs.c:75: __endasm;
	call 0xad5
;iocs.c:76: return;
	ret
_cls_end::
	.area _CODE
	.area _INITIALIZER
	.area _CABS (ABS)
