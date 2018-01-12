; this is an example of the z80 assembler
; it's some testing code for Z80 computer made by me

	org 0

	di
	ld b,255
	ldi
cycle	ld de,(0)	;delay 12.8 ms
	ld de,(0)
	ld de,(0)
	ld de,(0)
	ld de,(0)
	ld de,(0)
	ld de,(0)
	ld de,(0)
	ld de,(0)
	ld de,(0)
	djnz cycle
	
	ld a,128	;initialize 8255
	out (3),a
	ld a,183	;initialize CTC
	out (224),a	
	ld a,16
	out (224),a
	xor a
	out (224),a
	ld a,51
	out (225),a
	out (226),a
	out (227),a
	ld sp,16384
	ld a,1
	ld i,a
	im 2
	ei
c1	halt
	jr c1

sound	ld a,(data)	;sound routine, called on interrupt
	out (2),a
	xor 128
	ld (data),a
	reti

data	defb 128

	org 256		;interrupt table
	defw sound 
	defw sound
	defw sound
	defw sound
