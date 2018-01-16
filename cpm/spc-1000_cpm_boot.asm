;CP/M2.2 boot loader for SPC-1000
; by Miso Kim

;	include "spc-1000_all.inc"	
;   include "spc-1000_cpm.inc"

MSIZE	EQU	58		;mem size in kbytes
;
BIAS	EQU	(MSIZE-20)*1024	;offset from 20k system
CCP		EQU	3400H+BIAS	;base of the ccp
BIOS	EQU	CCP+1600H	;base of the bios
BIOSL	EQU	0300H		;length of the bios
BOOT	EQU	BIOS
SIZE	EQU	BIOS+BIOSL-CCP	;size of cp/m system
SECTS	EQU	SIZE/128	;# of sectors to load
DSKIX	EQU	0fa00h-33
CPM_RUN	EQU 0CB00H
CLR2    	EQU 00AD5H
CLS		EQU 1b42h

SDINIT		EQU 0
SDWRITE		EQU 1
SDREAD		EQU 2
SDSEND  	EQU 3
SDCOPY		EQU 4
SDFORMAT	EQU 5
SDSTATUS	EQU 6
SDDRVSTS	EQU 7
SDRAMTST	EQU 8
SDTRANS2	EQU 9
SDNOACT		EQU 0Ah
SDTRANS1	EQU 0Bh
SDRCVE		EQU 0Ch
SDGO		EQU 0Dh
SDLOAD		EQU 0Eh
SDSAVE		EQU 0Fh
SDLDNGO		EQU 010h

    ORG CPM_RUN
	
;	DEFB "SYS"
	
;CCP=a800-b005 (track0,sector2-track0,sector10)
;BDOS=b006-bdff (track0,sector10-track1,sector7)
;BIOS jump table and work area=be00-bfff (track1,sector8-9)
;BIOS program=c800-cdff (track1,sector10-15)

	LD	IX,DSKIX
	LD	SP,DSKIX
	call CLR2
LOADCPM:
	ld  d, 017h
	call SENDCOM
	ld  d, 3
	call SENDDAT
	ld	bc,0002h	;track0,sector2
	ld	h,15		;15 sectors
	ld	de,CCP
	call	LOAD1
	ld  bc,0101h
	ld  h,16
	call 	LOAD1
	ld  bc,0201h
	ld  h,2
	call 	LOAD1
	jp	BIOS
	
LOAD:
	push bc
	push hl
	ld h,1
	call LOAD1
	pop hl
	pop bc
	ld a,h
	dec a
	ret z
	ld h,a
	inc c
	ld a,16
	dec c
	jr nc, LOAD
	inc b
	jr LOAD
LOAD1:	
	PUSH HL
	PUSH DE
	PUSH BC
   	LD	D,SDREAD         	; 
   	CALL	SENDCOM      	;
   	LD	D,H         		; # of sectors
   	CALL	SENDDAT      	;
	LD  D,0					; drive
   	CALL	SENDDAT      	;
	POP  HL					;
   	LD	D,H         		; track
   	CALL	SENDDAT      	;
   	LD	D,L         		; sector
   	CALL	SENDDAT      	;
   	LD	D,SDSEND         	;
   	CALL	SENDCOM      	;
	POP HL
	POP BC
   	LD	C,000h         	;
SYM1:   
	CALL	GETDATA      	;
    LD	(HL),D         	;r
    INC	HL            	;#
    DEC	BC             	;
	LD A,B
	OR C
    JR	NZ,SYM1     	;
	LD D,H
	LD E,L
	RET
SENDCOM:
    LD	B,0C0h         	;
    LD	C,002h         	;
    SET 7,A
    OUT	(C),A         	;y
SENDDAT:	
    LD	B,0C0h         	;
    LD	C,002h         	;
CHKRFD1:   	IN	A,(C)          	;x
    AND	002h          	;
    JR	Z,CHKRFD1      	;(
    LD	C,002h         	;
    XOR	A             	;
    OUT	(C),A         	; ATN=0
    LD	C,000h         	;
    OUT	(C),D         	;
    LD	C,002h         	;
;    LD	A,010h         	;
	SET 4,A
    OUT	(C),A         	;
    LD	C,002h         	;
CHKDAC2:   	IN	A,(C)   ;x
    AND	004h          	;
    JR	Z,CHKDAC2      	;(
    LD	C,002h         	;
    XOR	A             	;
    OUT	(C),A         	;y
    LD	C,002h         	;
CHKDAC3:   	IN	A,(C)          	;x
    AND	004h          	;
    JR	NZ,CHKDAC3     	; 
    RET               	;
GETDATA:
    PUSH	BC           	;
    LD	C,002h         	;
    LD	B,0C0h         	;
    LD	A,020h         	;> 
	SET 5,A
    OUT	(C),A         	;y
    LD	C,002h         	;
CHKDAV0:   	IN	A,(C)          	;x
    AND	001h          	;
    JR	Z,CHKDAV0      	;(
    LD	C,002h         	;
    XOR	A             	;
    OUT	(C),A         	;y
    LD	C,001h         	;
    IN	D,(C)          	;P
    LD	C,002h         	;
;    LD	A,040h         	;>@
	SET 6,A
    OUT	(C),A         	;y
    LD	C,002h         	;
CHKDAV1:   	IN	A,(C)          	;x
    AND	001h          	;
    JR	NZ,CHKDAV1     	; 
    LD	C,002h         	;
    XOR	A             	;
    OUT	(C),A         	;y
    POP	BC            	;
    RET               	;	
	END