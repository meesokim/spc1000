;==============================================================================
;   Floppy Disk access code for SPC-1000
;
;           sd725.s
;                                   By meeso.kim
;==============================================================================

	.module	sd725
	.area	_CODE
	
SDINIT		= 0
SDWRITE		= 1
SDREAD		= 2
SDSEND  	= 3
SDCOPY		= 4
SDFORMAT	= 5
SDSTATUS	= 6
SDDRVSTS	= 7
SDRAMTST	= 8
SDTRANS2	= 9
SDNOACT		= 0xA
SDTRANS1	= 0xB
SDRCVE		= 0xC
SDGO		= 0xD
SDLOAD		= 0xE
SDSAVE		= 0xF
SDLDNGO		= 0x10	
CLR2		= 0xad5
		
_chkfdd::
	push ix
	ld ix,#2
	add ix,sp
	ld a,(ix)
	
	pop ix
	ret
	
_sd_init::
CHECKFDD:
	LD	B,#0xC0         	
  	LD	C,#0x02         
   	XOR	A             	
  	OUT	(C),A         	
   	LD	C,#0         	
   	OUT	(C),A         	
	ld d, #SDINIT
	call sendcmd
	ld d, #SDDRVSTS
	call sendcmd
	call recvdata
	ld a, d
	and #0x10
	jr z, CHECKFDD
	ret

_sd_read::
	push bc
	push ix
	ld ix,#6
	add ix,sp
	ld d, #SDREAD
	call sendcmd
	ld d,0(ix)
	call senddata
	ld d,1(ix)
	call senddata
	ld d,2(ix)
	call senddata
	ld d,3(ix)
	call senddata
	ld d, #SDSEND
	call sendcmd
	ld l,4(ix)
	ld h,5(ix)
	ld b,0(ix) 
	ld c,#0x0
RDLOOP:
	call recvdata
	ld (hl), d
	inc hl
	dec bc
	ld  a, b
	or  c
	jr nz, RDLOOP
	pop ix
	pop bc
	ret
	
_sd_write::
	push bc
	push ix
	ld ix,#6
	add ix,sp
	ld d, #SDWRITE
	call sendcmd
	ld d,0(ix)
	call senddata
	ld d,1(ix)
	call senddata
	ld d,2(ix)
	call senddata
	ld d,3(ix)
	call senddata
	ld l,4(ix)
	ld h,5(ix)
	ld b,0(ix)
	ld c,#0
WRLOOP:	
	ld d,(hl)
	push bc
	call senddata
	pop bc
	inc hl
	dec bc
	ld a, b
	or c
	jr nz, WRLOOP
	pop ix
	pop bc
	ret
	
_sd_format::
	push ix
	ld ix,#4
	add ix,sp
	ld d, #SDFORMAT
	call sendcmd
	ld d,(ix)
	call senddata
	pop ix
	ret
	
_sd_drvstate::
	ld d, #SDDRVSTS
	call sendcmd
	call recvdata
	ld l, d
	ret
	
_sd_sendstate::
	ld d, #SDSTATUS
	call sendcmd
	call recvdata
	ld l, d
	ret
	
_sd_load::
	push hl
	push de
	push bc
	ld 	d, #SDREAD
	call sendcmd
	ld	d, h
	call senddata
	ld	d, #0
	call senddata
	pop hl
	ld	d, h
	call senddata
	ld  d, l
	call senddata
	ld  d, #SDSEND
	call sendcmd
	pop hl
	pop bc
	ld  c,#0
RDLOOPx:
	call recvdata
	ld (hl), d
	inc hl
	dec bc
	ld  a, b
	or  c
	jr nz, RDLOOPx
	ret	

sendcmd:
    LD	B,#0xC0         	
    LD	C,#0x02         	
    LD	A,#0x80         	
    OUT	(C),A         	
senddata:	
    LD	B,#0xC0         	
    LD	C,#0x02         	
CHKRFD1:   	
	IN	A,(C)          	
    AND	#0x02          	
    JR	Z,CHKRFD1      	
    LD	C,#0x02         	
    XOR	A             	
    OUT	(C),A         	
    LD	C,#0x00         	
    OUT	(C),D         	
    LD	C,#0x02         	
    LD	A,#0x10         	
    OUT	(C),A         	
    LD	C,#0x02         
CHKDAC2:   	
	IN	A,(C)   
    AND	#0x04          	
    JR	Z,CHKDAC2      	
    LD	C,#0x02         
    XOR	A             
    OUT	(C),A         	
    LD	C,#0x02         
CHKDAC3:   	
	IN	A,(C)          
    AND	#0x04          	
    JR	NZ,CHKDAC3     	
    RET               
	
recvdata:
    PUSH	BC           
    LD	C,#0x02         	
    LD	B,#0xC0         	
    LD	A,#0x20         	
    OUT	(C),A         	
    LD	C,#0x02         	
CHKDAV0:   	
	IN	A,(C)          	
    AND	#0x01          	
    JR	Z,CHKDAV0      	
    LD	C,#0x02         
    XOR	A             	
    OUT	(C),A         	
    LD	C,#0x01         	
    IN	D,(C)          	
    LD	C,#0x02         	
    LD	A,#0x40         	
    OUT	(C),A         
    LD	C,#0x02         	
CHKDAV1:   	
	IN	A,(C)          	
    AND	#0x01          	
    JR	NZ,CHKDAV1     	
    LD	C,#0x02         
    XOR	A             	
    OUT	(C),A         	
    POP	BC            	
    RET               	
