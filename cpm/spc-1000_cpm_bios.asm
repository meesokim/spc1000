;	Z80 CBIOS for SPC-1000
;
;	Copyright (C) 1988-2007 by Udo Munk
;	Copyright (C) 2014 by Miso Kim
;
MSIZE	EQU	58		;cp/m version memory size in kilobytes
;	include "spc-1000_all.inc"
;ACCPRT  	EQU 00864H
;ASCGET  	EQU 00C62H
;ACCOUT  	EQU 008D9H
;ACCDIS  	EQU 008DEH
;GACPRT  	EQU 0094FH
;CR1     	EQU 00843H
;CR2     	EQU 00823H
KEYBUF  	EQU 07A4FH
;
;	"bias" is address offset from 3400H for memory systems
;	than 16K (referred to as "b" throughout the text).
;
BIAS	EQU	(MSIZE-20)*1024
CCP	    EQU	3400H+BIAS	;base of ccp
BDOS	EQU	CCP+806H	;base of bdos
BIOS	EQU	CCP+1600H	;base of bios
NSECTS	EQU	(BIOS-CCP)/128	;warm start sector count
CDISK	EQU	0004H		;current disk number 0=A,...,15=P
IOBYTE	EQU	0003H		;intel i/o byte
;
;	I/O ports
;
CONSTA	EQU	0		;console status port
CONDAT	EQU	1		;console data port
PRTSTA	EQU	2		;printer status port
PRTDAT	EQU	3		;printer data port
AUXDAT	EQU	5		;auxiliary data port
FDCD	EQU	10		;fdc-port: # of drive
FDCT	EQU	11		;fdc-port: # of track
FDCS	EQU	12		;fdc-port: # of sector
FDCOP	EQU	13		;fdc-port: command
FDCST	EQU	14		;fdc-port: status
DMAL	EQU	15		;dma-port: dma address low
DMAH	EQU	16		;dma-port: dma address high
;
	ORG	BIOS		;origin of this program
;
;	jump vector for individual subroutines
;
	JP	BOOT		;cold start
WBOOTE: JP	WBOOT		;warm start
	JP	CONST		;console status
	JP	CONIN		;console character in
	JP	CONOUT		;console character out
	JP	LIST		;list character out
	JP	PUNCHS		;punch character out
	JP	READER		;reader character in
	JP	HOME		;move head to home position
	JP	SELDSK		;select disk
	JP	SETTRK		;set track number
	JP	SETSEC		;set sector number
	JP	SETDMA		;set dma address
	JP	READ		;read disk
	JP	WRITE		;write disk
	JP	LISTST		;return list status
	JP	SECTRAN		;sector translate
;
;	fixed data tables for four-drive standard
;	IBM-compatible 8" SD disks
;
;	disk parameter header for disk 00
DPBASE:	
	DEFW	0000H,0000H
	DEFW	0000H,0000H
	DEFW	DIRBF,DPBLK
	DEFW	CHK00,ALL00
;	disk parameter header for disk 01
	DEFW	0000H,0000H
	DEFW	0000H,0000H
	DEFW	DIRBF,DPBLK
	DEFW	CHK01,ALL01
;
;	sector translate vector for the IBM 8" SD disks
;
TRANS:	
	DEFB	1,7,13,19	;sectors 1,2,3,4
	DEFB	25,5,11,17	;sectors 5,6,7,8
	DEFB	23,3,9,15	;sectors 9,10,11,12
	DEFB	21,2,8,14	;sectors 13,14,15,16
	DEFB	20,26,6,12	;sectors 17,18,19,20
	DEFB	18,24,4,10	;sectors 21,22,23,24
	DEFB	16,22		;sectors 25,26
;
;	disk parameter block, SD725 (from TF-20)
;
DPBLK:  
	DEFW	32		;sectors per track
	DEFB	4		;block shift factor
	DEFB	15		;block mask
	DEFB	1		;extent mask
	DEFW	153		;disk size-1
	DEFW	127		;directory max
	DEFB	0C0H	;alloc 0
	DEFB	0		;alloc 1
	DEFW	32		;check size
	DEFW	3		;track offset
;
;	messages
;
SIGNON: 
	DEFM	'58K CP/M Version 2.2'
	DEFB	13,10
	DEFM	'(for SPC-1000 with SD-725)'
	DEFB	13,10,0
;
LDERR:	
	DEFM	'BIOS: error booting'
	DEFB	13,10,0

;
;	end of fixed tables
;
;	utility functions
;
;	print a 0 terminated string to console device
;	pointer to string in HL
;
PRTMSG:	LD	A,(HL)
	OR	A
	RET	Z
	LD	C,A
	CALL	CONOUT
	INC	HL
	JP	PRTMSG
;
;	individual subroutines to perform each function
;	simplest case is to just perform parameter initialization
;
BOOT:   
	LD	SP,80H		;use space below buffer for stack
	XOR A
	LD  BC, 06000h
	OUT (C), A
	ld  a, 8eh
	LD  BC, 02000h
	OUT (C), a
	DEC BC
CLS3:	
	XOR A
	OUT (C), A
	DEC BC
	LD  A,B
	OR  C
	JR  NZ, CLS3
	LD	HL,SIGNON	;print message
	CALL	PRTMSG
	XOR	A		;zero in the accum
	LD	(IOBYTE),A	;clear the iobyte
	LD	(CDISK),A	;select disk zero
	JP	GOCPM		;initialize and go to cp/m
;
;	simplest case is to read the disk until all sectors loaded
;
WBOOT:  LD	SP,80H		;use space below buffer for stack
	LD	C,0		;select disk 0
	CALL	SELDSK
	CALL	HOME		;go to track 00
;
	LD	B,NSECTS	;b counts # of sectors to load
	LD	C,0		;c has the current track number
	LD	D,4		;d has the next sector to read
;	note that we begin by reading track 0, sector 2 since sector 1
;	contains the cold start loader, which is skipped in a warm start
	LD	HL,CCP		;base of cp/m (initial load point)
LOAD1:				;load one more sector
	PUSH	BC		;save sector count, current track
	PUSH	DE		;save next sector to read
	PUSH	HL		;save dma address
	LD	C,D		;get sector address to register c
	CALL	SETSEC		;set sector address from register c
	POP	BC		;recall dma address to b,c
	PUSH	BC		;replace on stack for later recall
	CALL	SETDMA		;set dma address from b,c
;	drive set to 0, track set, sector set, dma address set
	CALL	READ
	OR	A		;any errors?
	JP	Z,LOAD2		;no, continue
	LD	HL,LDERR	;error, print message
	CALL	PRTMSG
	DI			;and halt the machine
	HALT
;	no error, move to next sector
LOAD2:	POP	HL		;recall dma address
	LD	DE,128		;dma=dma+128
	ADD	HL,DE		;new dma address is in h,l
	POP	DE		;recall sector address
	POP	BC		;recall number of sectors remaining,
				;and current trk
	DEC	B		;sectors=sectors-1
	JP	Z,GOCPM		;transfer to cp/m if all have been loaded
;	more sectors remain to load, check for track change
	INC	D
	LD	A,D		;sector=27?, if so, change tracks
	CP	31
	JP	C,LOAD1		;carry generated if sector<27
;	end of current track, go to next track
	LD	D,1		;begin with first sector of next track
	INC	C		;track=track+1
;	save register state, and change tracks
	CALL	SETTRK		;track address set from register c
	JP	LOAD1		;for another sector
;	end of load operation, set parameters and go to cp/m
GOCPM:
	LD	A,0C3H		;c3 is a jmp instruction
	LD	(0),A		;for jmp to wboot
	LD	HL,WBOOTE	;wboot entry point
	LD	(1),HL		;set address field for jmp at 0
;
	LD	(5),A		;for jmp to bdos
	LD	HL,BDOS		;bdos entry point
	LD	(6),HL		;address field of jump at 5 to bdos
;
	LD	BC,80H		;default dma address is 80h
	CALL	SETDMA
;
	LD	A,(CDISK)	;get current disk number
	LD	C,A		;send to the ccp
	JP	CCP		;go to cp/m for further processing
;
;
;	simple i/o handlers
;
;	console status, return 0ffh if character ready, 00h if not
;
CONST:	
;    IN	A,(CONSTA)	;get console status
	LD A, (KEYBUF)
	OR A
	RET NZ
	LD A, 0ffh
	RET
;
;	console character into register a
;
CONIN:	
;	IN	A,(CONDAT)	;get character from console
	CALL ASCGET1		; SPC-1000 IOCS
	RET
;
;	console character output from register c
;
CONOUT: 
	LD	A,C		;get to accumulator
;	OUT	(CONDAT),A	;send character to console
;	CP 13
;	JR NZ, ASCPRT
;	CALL CR2
;	RET
ASCPRT:
	PUSH HL
	CALL ACCPRT1
	POP  HL
	RET
;
;	list character from register c
;
LIST:	LD	A,C		;character to register a
;	OUT	(PRTDAT),A
	
	RET
;
;	return list status (00h if not ready, 0ffh if ready)
;
LISTST: 
;	IN	A,(PRTSTA)
	LD	A, 0
	RET
;
;	punch character from register c
;
PUNCHS:	
    LD	A,C		;character to register a
;	OUT	(AUXDAT),A
	RET
;
;	read character into register a from reader device
;
READER: 
;	IN	A,(AUXDAT)
	RET
;
;
;	i/o drivers for the disk follow
;
;	move to the track 00 position of current drive
;	translate this call into a settrk call with parameter 00
;
HOME:	LD	C,0		;select track 0
	JP	SETTRK		;we will move to 00 on first read/write
;
;	select disk given by register C
;
SELDSK: LD	HL,0000H	;error return code
	LD	A,C
	CP	2		;FD drive 0-1?
	JR	C,SELFD		;go
	RET			;no, error
;	disk number is in the proper range
;	compute proper disk parameter header address
SELFD:
    LD  C,A
    LD  A,(FDD#D)
	CP  C
	JR  Z,SELFD$ 
	LD HL,CHGFLG
	SET 2,(HL)
SELFD$:	
	LD H,0
	LD (FDD#D),A
	LD	L,A		;L=disk number 0,1,2,3
	ADD	HL,HL		;*2
	ADD	HL,HL		;*4
	ADD	HL,HL		;*8
	ADD	HL,HL		;*16 (size of each header)
	LD	DE,DPBASE
	ADD	HL,DE		;HL=.dpbase(diskno*16)
	RET
;
;	set track given by register c
;
SETTRK: 
;	LD	A,C
;	OUT	(FDCT),A
	LD A,(FDD#T)
	CP C
	RET Z
	LD HL,CHGFLG
	SET 1,(HL)
	LD	A,C
	LD (FDD#T),A
	RET
;
;	set sector given by register c
;
SETSEC: 
;	LD	A,C
;	OUT	(FDCS),A
    DEC C
	LD B,C
	XOR A
	LD A,(FDD#S)
	RR A
	OR A
	RR C
	CP C
	JR Z, SETSEC0
	LD HL,CHGFLG
	SET 0,(HL)
SETSEC0:	
	LD	A,B
	LD (FDD#S),A
	RET
;
;	translate the sector given by BC using the
;	translate table given by DE
;
SECTRAN:
	LD	A,D		;do we have a translation table?
	OR	E
	JP	NZ,SECT1	;yes, translate
	LD	L,C		;no, return untranslated
	LD	H,B		;in HL
	INC	L		;sector no. start with 1
	RET	NZ
	INC	H
	RET
SECT1:	EX	DE,HL		;HL=.trans
	ADD	HL,BC		;HL=.trans(sector)
	LD	L,(HL)		;L = trans(sector)
	LD	H,0		;HL= trans(sector)
	RET			;with data in HL
;
;	set dma address given by registers b and c
;
SETDMA: 
;	LD	A,C		;low order address
;	OUT	(DMAL),A
;	LD	A,B		;high order address
;	OUT	(DMAH),A	;in dma
	LD (DATADDR), BC
	RET
;
;	perform read operation
;
READ:	
;	XOR	A		;read command -> A
;	JP	WAITIO		;to perform the actual i/o
	LD A,(CHGFLG)
	CP 0
	JR NZ, REALREAD
BUFCHECK:
	LD A,(FDD#S)
	RRA
	JR BUFCOPY
REALREAD:	
	LD	D, SDREAD
	CALL	SENDCOM
	LD	HL,FDD#N	; FDD#N
	LD	D,(HL)
	LD  B,D
	LD  C,0
	PUSH BC			
	CALL	SENDDAT
	INC	HL 			; FDD#D
	LD	D,(HL)
	CALL	SENDDAT
	INC	HL			; FDD#T
	LD	D,(HL)
	CALL	SENDDAT
	INC	HL			; FDD#S
	LD	A,(HL)
	RRA
	LD  D,A
	INC D
	CALL	SENDDAT
	LD  D,03h
	CALL    SENDCOM
	POP BC
	LD	HL,DISKDATA
SYM1:   
	CALL GETDATA ; from FDD  
    LD (HL),D   ; and write to RAM  
    INC HL      ; from 0CC00h   
    DEC BC      ;  
    LD A,B      ;  
    OR C        ;  
    JR NZ, SYM1 ; ------------------  
BUFCOPY:
	LD  HL,DISKDATA
	LD  BC,128
	JR NC, EVENREAD
ODDREAD:
	ADD HL,BC
EVENREAD:	
	LD  DE,(DATADDR)
	LDIR
	XOR A
	LD (CHGFLG), A
	RET
;
;	perform a write operation
;
WRITE:	
;	LD	A,1		;write command -> A
WRITEC:				; C == 2
	LD  HL,DISKDATA
	LD  A,(FDD#S)
	OR A
	RRA
	LD  BC, 128
	JR  NC, EVENWRIT
ODDWRIT:
	ADD HL,BC
EVENWRIT:
	LD  DE,(DATADDR)
	EX  DE,HL
	LDIR
CHKWCPY:
CHKWFLG:
WRITEM:
	LD	D, SDWRITE
	CALL	SENDCOM
	LD	HL,FDD#N
	LD	D,(HL)
	CALL	SENDDAT
	INC	HL
	LD	D,(HL)
	CALL	SENDDAT
	INC	HL
	LD	D,(HL)
	CALL	SENDDAT
	INC	HL
	LD	A,(HL)
	OR A
	RR A
	LD  D,A
	INC D
	CALL	SENDDAT
	LD	HL,DISKDATA
	LD	D,H
	CALL	SENDDAT
	LD  D,L
	CALL	SENDDAT
	XOR A
	RET
	
READ1:	                                ;                     354    368 ;	JP	WAITIO		;to perform the actual i/o
    LD  D, SDLOAD                       ;e3aa  16 0e          355    369 
    CALL    SENDCOM                     ;e3ac  cd 01 e4       356    370 
    LD  HL,FDD#N                        ;e3af  21 67 e4       357    371 
    LD  D,(HL)                          ;e3b2  56             358    372 
    CALL    SENDDAT                     ;e3b3  cd 09 e4       359    373 
    INC HL                              ;e3b6  23             360    374 
    LD  D,(HL)                          ;e3b7  56             361    375 
    CALL    SENDDAT                     ;e3b8  cd 09 e4       362    376 
    INC HL                              ;e3bb  23             363    377 
    LD  D,(HL)                          ;e3bc  56             364    378 
    CALL    SENDDAT                     ;e3bd  cd 09 e4       365    379 
    INC HL                              ;e3c0  23             366    380 
    LD  D,(HL)                          ;e3c1  56             367    381 
    CALL    SENDDAT                     ;e3c3  cd 09 e4       369    383 
    LD  HL,DATADDR+1                    ;e3c6  21 6c e4       370    384 
    LD  D,(HL)                          ;e3c9  56             371    385 
    CALL    SENDDAT                     ;e3ca  cd 09 e4       372    386 
    DEC HL                              ;e3cd  2b             373    387 
    LD  D,(HL)                          ;e3ce  56             374    388 
    CALL    SENDDAT                     ;e3cf  cd 09 e4       375    389 
    RET                                 ;e3d2  c9             376    390 

;
;	enter here from read and write to perform the actual i/o
;	operation.  return a 00h in register a if the operation completes
;	properly, and 01h if an error occurs during the read or write
;
WAITIO: 
;   OUT	(FDCOP),A	;start i/o operation
;	IN	A,(FDCST)	;status of i/o operation -> A
	RET

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

		
;
;   SPC-1000 FDD command I/O
;
SENDCOM:
    LD	B,0C0h         	;
    LD	C,002h         	;
    LD	A,080h         	;
    OUT	(C),A         	;
SENDDAT:	
    LD	B,0C0h         	;
    LD	C,002h         	;
CHKRFD1:   	IN	A,(C)   ;
    AND	002h          	;
    JR	Z,CHKRFD1      	;
    LD	C,002h         	;
    XOR	A             	;
    OUT	(C),A         	; ATN=0
    LD	C,000h         	;
    OUT	(C),D         	;
    LD	C,002h         	;
    LD	A,010h         	;
    OUT	(C),A         	;
    LD	C,002h         	;
CHKDAC2:   	IN	A,(C)   ;
    AND	004h          	;
    JR	Z,CHKDAC2      	;
    LD	C,002h         	;
    XOR	A             	;
    OUT	(C),A         	;
    LD	C,002h         	;
CHKDAC3:   	IN	A,(C)          	;
    AND	004h          	;
    JR	NZ,CHKDAC3     	; 
    RET               	;
GETDATA:
    PUSH	BC           	;
    LD	C,002h         	;
    LD	B,0C0h         	;
    LD	A,020h         	;
    OUT	(C),A         	;
    LD	C,002h         	;
CHKDAV0:   	IN	A,(C)          	;
    AND	001h          	;
    JR	Z,CHKDAV0      	;
    LD	C,002h         	;
    XOR	A             	;
    OUT	(C),A         	;
    LD	C,001h         	;
    IN	D,(C)          	;
    LD	C,002h         	;
    LD	A,040h         	;
    OUT	(C),A         	;
    LD	C,002h         	;
CHKDAV1:   	IN	A,(C)   ;
    AND	001h          	;
    JR	NZ,CHKDAV1     	; 
    LD	C,002h         	;
    XOR	A             	;
    OUT	(C),A         	;
    POP	BC            	;
    RET               	;
	
FDD#N:	DEFB	1
FDD#D:	DEFB	0
FDD#T:	DEFB	0
FDD#S:	DEFB	0
DATADDR:DEFW	0
CHGFLG: DEFB    1
PRESEC: DEFB 	0xffh	
	
BRKEY:  LD  A,080h                    
    IN  A,(0)                         
    AND 012h                          
    RET                               
                                      ;
ASCGET1: PUSH    BC                    
    PUSH    DE                        
    PUSH    HL                        
    LD  HL,(POINT1)                   
    LD  A,L                           
    CP  H                             
    JR  Z,BUFSET                      
BUFCEK: INC A                         
    AND 03Fh                          
    LD  (POINT1),A                    
    LD  L,A                           
    LD  H,0                           
    LD  DE,INKYBF                     
    ADD HL,DE                         
    LD  A,(HL)                        
GETRET: POP HL                        
    POP DE                            
    POP BC                            
    RET                               
                                      ;
BUFSET: DEFB    03Eh                  ; LD A,
FLASHF: DEFB    001h                  ;
    OR  A                             
    JR  NZ,FLGET                      
    CALL    KEYSEN                    
    CALL    BRKEY                     
    JR  NZ,KEYGT                      
BRKGTR: CALL    NEWFLG                
    LD  A,3                           
    JR  GETRET                        
KEYGT:  CALL    KEYSEN                
    JR  Z,ZERORT                      
    CALL    NEWCEK                    
    JR  NZ,NEWKYS                     
    CALL    SAMECK                    
    JR  Z,REPETS                      
ZERORT: CALL    NEWFLG                
ZEROR2: XOR A                         
    JR  GETRET                        
FLGET:  CALL    FLSHGT      
    LD  C,A                           
    LD  A,080h                        
    IN  A,(0)                         
    AND 012h                          
    JR  Z,BRKGTR                      
    LD  A,C                           
    OR  A                             
    JR  Z,NEWKYS                      
REPETS: CALL    NEWFLG                
    DEFB    021h                      ; LD HL,(KEYBFA)
KEYBFA: DEFW    0FFFEh                ;
    DEFB    03Eh                      ; LD A,(KEYBFD)
KEYBFD: DEFB    000h                  
    LD  (HL),A                        
    LD  HL,30                         
    JR  KEYST2                        
NEWKYS: CALL    NEWFLG                
    DEFB    021h                      ; LD HL,300
REPTSW: DEFW    200                   ; auto repeat key duration
KEYST2: LD  (REPETF),HL               
    LD  HL,NEWBUF                     
    LD  D,0                           
    LD  C,9                           
SETLP1: LD  B,8                       
    LD  A,(HL)                        
    CPL                               
    OR  A                             
    JR  Z,SETPL3                      
SETPL2: RRCA                          
    JP  C,KYBFST                      
    INC D                             
    DJNZ    SETPL2                    
    JR  SETPL4                        
SETPL3: LD  A,D                       
    ADD A,8                           
    LD  D,A                           
SETPL4: INC HL                        
    DEC C                             
    JR  NZ,SETLP1                     
KYBFSR: LD  HL,(POINT1)               
    LD  A,L                           
    CP  H                             
    JP  Z,ZEROR2                      
    JP  BUFCEK                        
                                      ;
FLASHS: 
;	 CALL    ADRCAL                  ;
;    SET 3,B                           
;    LD  (FLASHA),BC                   
;    IN  A,(C)                         
;    LD  (FLASHD),A                    
;    OR  1                             
;    OUT (C),A                         
;    LD  (FLASHO),A                    
;    LD  A,(TICONT)                    
;    LD  (FLASHC+1),A                 ;??+1"??빠져?�었??
    RET                               ;
                                      ;
                                      ;
FLASHG: DEFB    001h                  
FLASHA: DEFW    0                     
    DEFB    03Eh                      ; LD A,
FLASHD: DEFB    000h                  ;
;    AND 0FEh                          
;    LD  E,A                           
;    LD  A,(TICONT)                    
FLASHC: SUB 0                         ; cursor blink time
    AND 010h                          
    LD  A,001h                        
    JR  Z,L0D28                       
    XOR A                             
L0D28:  OR  E                         
    DEFB    0FEh                      ;CP
FLASHO: DEFB    000h                  
    RET Z                             
;    OUT (C),A                         
;    LD  (FLASHO),A                    
    RET                               
                                      ;
FLASHE: LD  BC,(FLASHA)               
    LD  A,(FLASHD)                    
;    OUT (C),A                         
    RET                               
                                      ;
                                      ;
FLSHGT: PUSH    BC                    
    PUSH    DE                        
    PUSH    HL                        
    CALL    FLASHS                    
FLGTLP: CALL    FLASHG                
    CALL    KEYSEN                    
    JR  Z,FLSHL3                      
    CALL    NEWCEK                    
    LD  A,0                           
    JR  NZ,FLSHRT                     
    CALL    SAMECK                    
    JR  NZ,FLGTLP                     
    DEFB    021h                      ; LD HL,
REPETF: DEFW    00000h                ;
    DEC HL                            ;
    LD  (REPETF),HL                   
    LD  A,H                           
    OR  L                             
    JR  NZ,FLGTLP                     
    LD  A,1                           
    JR  FLSHRT                        
FLSHL3: CALL    NEWFLG                
FLGTL2: CALL    FLASHG                
    CALL    KEYSEN                    
    JR  Z,FLGTL2                      
    XOR A                             
FLSHRT: PUSH    AF                    
    CALL    FLASHE                    
    POP AF                            
    POP HL                            
    POP DE                            
    POP BC                            
    RET                               
                                      ;
                                      ;
                                      ;
KEYSEN: CALL    KEYSN2                
KEYSN0: LD  B,4                       
KEYSN1: CALL    KEYSN2                
    BIT 1,E                           
    JR  NZ,KEYSN0                     
    DJNZ    KEYSN1                    
    LD  A,E                           
    AND 1                             
    RET                               
                                      ;
KEYSN2: LD  HL,NEWBUF                 
    PUSH    BC                        
    LD  BC,08009h                     ;KEYIO
    LD  DE,00900h                     
KEYSN3: IN  A,(C)                     
    XOR (HL)                          
    JR  Z,L0D9D                       
    SET 1,E                           
L0D9D:  XOR (HL)                      
    LD  (HL),A                        
    CPL                               
    OR  A                             
    JR  Z,L0DA5                       
    SET 0,E                           
L0DA5:  INC HL                        
    DEC BC                            
    DEC D                             
    JR  NZ,KEYSN3                     
    IN  A,(C)                         
    XOR (HL)                          
    JR  Z,L0DB1                       
    SET 1,E                           
L0DB1:  XOR (HL)                      
    LD  (HL),A                        
    AND 012h                          
    JR  NZ,L0DB9                      
    SET 0,E                           
L0DB9:  POP BC                        
    RET                               
                                      ;
NEWCEK: LD  HL,OLDBUF                 
    LD  DE,NEWBUF                     
    LD  B,9                           
NEWPL1: LD  A,(DE)                    
    CPL                               
    AND (HL)                          
    RET NZ                            
    INC DE                            
    INC HL                            
    DJNZ    NEWPL1                    
    LD  A,(DE)                        
    CPL                               
    AND (HL)                          
    AND 010h                          
    RET                               
                                      ;
SAMECK: LD  HL,OLDBUF                 
    LD  DE,NEWBUF                     
    LD  B,10                          
SAMELP: LD  A,(DE)                    
    XOR (HL)                          
    RET NZ                            
    INC DE                            
    INC HL                            
    DJNZ    SAMELP                    
    RET                               
                                      ;
NEWFLG: LD  HL,NEWBUF                 
    LD  DE,OLDBUF                     
    LD  B,9                           
FLGLP1: LD  A,(DE)                    
    CPL                               
    OR  (HL)                          
    LD  C,(HL)                        
    LD  (HL),A                        
    LD  A,C                           
    LD  (DE),A                        
    INC DE                            
    INC HL                            
    DJNZ    FLGLP1                    
    LD  A,(HL)                        
    LD  (DE),A                        
    LD  DE,NEWBUF                     
    LD  HL,OLDBUF                     
    LD  BC,00900h                     
FLGLP2: LD  A,(DE)                    
    CPL                               
    OR  A                             
    JR  Z,FLGLP3                      
    DEC C                             
    JR  NZ,FLGLP5                     
    OR  (HL)                          
    LD  (HL),A                        
    LD  A,0FFh                        
    LD  (DE),A                        
FLGLP4: LD  C,1                       
FLGLP3: INC DE                        
    INC HL                            
    DJNZ    FLGLP2                    
    RET                               
FLGLP5: PUSH    BC                    
    LD  BC,00801h                     
FLGLP6: RRCA                          
    JR  C,FLGLP7                      
    RLC C                             
    DJNZ    FLGLP6                    
    POP BC                            
    JR  FLGLP4                        
FLGLP9: RRCA                          
    CALL    C,FLGLP8                  
FLGLP7: RLC C                         
    DJNZ    FLGLP9                    
    POP BC                            
    JR  FLGLP4                        
                                      ;
FLGLP8: PUSH    AF                    
    LD  A,C                           
    OR  (HL)                          
    LD  (HL),A                        
    LD  A,(DE)                        
    OR  C                             
    LD  (DE),A                        
    POP AF                            
    RET                               
                                      ;
                                      ;
KYBFST: LD  A,(HL)                    
    LD  (KEYBFA),HL                   
    LD  (KEYBFD),A                    
    LD  C,D                           
    LD  B,0                           
    LD  HL,KEYDT1                     ;NORMAL DATA
    LD  A,(NEWMOD)                    
    BIT 1,A                           
    JR  NZ,L0E4C                      
    LD  HL,KEYDT2                     ;SHIFT DATA
L0E4C:  ADD HL,BC                     
    LD  C,(HL)                        
    LD  B,A                           
    BIT 2,B                           
    JR  NZ,CTRLNO                     
    LD  A,C                           
    SUB 040h                          
    JR  C,NONKE?                      
    AND 01Fh                          
    JR  SETKY                         
CTRLNO: BIT 6,B                       
    JR  NZ,SETKY2                     
    LD  A,C                           
    SUB 040h                          
    JR  C,NONKEY                      
    AND 01Fh                          
    ADD A,080h                        
SETKY:  CALL    BFSTSB                
NONKEY: JP  KYBFSR                    
NONKE?: ADD A,010h                    
    CP  004h                          
    JR  NC,NONKEY                     
;   LD  (COLORF),A                    
    JR  NONKEY                        
                                      ;
                                      ;
SETKY2: LD  A,C                       
    CP  0F0h                          
    JR  NC,FUNCTN                     
    LD  A,(LOCKMD)                    
    OR  A                             
    LD  A,C                           
    JR  Z,SETKY                       
    CP  041h                          
    JR  C,SETKY                       
    AND 01Fh                          
    CP  01Bh                          
    LD  A,C                           
    JR  NC,SETKY                      
    XOR 020h                          
    JR  SETKY                         
FUNCTN: SUB 0F1h                      
    LD  L,A                           
    LD  H,0                           
    ADD HL,HL                         
    ADD HL,HL                         
    ADD HL,HL                         
;    ADD HL,HL                         
    LD  BC,FUNBUF                     
    ADD HL,BC                         
    LD  B,(HL)                        
    INC HL                            
    LD  A,B                           
    OR  A                             
    JR  Z,NONKEY                      
FUNCLP: LD  A,(HL)                    
    CALL    BFSTSB                    
    INC HL                            
    DJNZ    FUNCLP                    
    JR  NONKEY                        
BFSTSB: PUSH    HL                    
    PUSH    BC                        
    LD  C,A                           
    LD  HL,(POINT1)                   
    LD  A,H                           
    INC A                             
    AND 03Fh                          
    CP  L                             
    JR  Z,NONSET                      
    LD  (POINT2),A                    
    LD  L,A                           
    LD  H,0                           
    LD  A,C                           
    LD  BC,INKYBF                     
    ADD HL,BC                         
    LD  (HL),A                        
NONSET: POP BC                        
    POP HL                            
NORET:  RET                           



CTL:
	ld	hl,CTLTBL
	add	a,a
	add	a,l
	ld	l,a
	jr	nc,NOINC1
	inc	h
NOINC1:
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ex	de,hl
JPHL:
	jp	(hl)

CTLTBL:
	dw	CTLNUL,CTLNUL,CTLNUL,CTLNUL,CTLNUL,CTLNUL,CTLNUL,CTLNUL
	dw	CTLLFT,CTLRHT,CTLLF, CTLUP, CTLRHT,CTLCR, CTLNUL,CTLNUL
	dw	CTLNUL,CTLNUL,CTLNUL,CTLNUL,CTLNUL,CTLNUL,CTLNUL,CTLNUL
	dw	CTLNUL,CTLNUL,CTLCLS,CTLESC,CTLRHT,CTLLFT,CTLHOM,CTLNUL
	
	
ACCPRT1:
	ld	a,(ESCMOD)
	or	a
	jp	nz,ESC
	ld	a,c
	and	7fh		;reset bit7
	cp	20h
	jr	c,CTL
	call	COPYCHR
	ret	nz
	
CTLLF:				;0ah(LF), line feed + carriage return
	ld	hl,CSRX
	ld	(hl),0
	inc	hl
	inc	(hl)		;CSRY
	ld	a,(HEIGHT)
	cp	(hl)
	jr	nz,SETAD
	dec	(hl)
	ld	de,VDISP
	ld	hl,VDISP+64
	ld	bc,64*(24-1)
	
	ldir
	ld	a,(WIDTH)
	ld	b,a
LOWEST:
	ld	a," "
	ld	(de),a
	inc	de
	djnz	LOWEST

	ld  bc, VRAM+256
	ld  de, 32*8*23
	push de
MOVEN1:
	in  a, (C)
	dec  b
	out (c), a
	inc  b
	inc  bc
	dec  de
	ld  a,d
	or  e
	jr nz, MOVEN1
	ld  h,0
	pop bc
MOVEN2:
	xor a
	out (c), a
	inc bc
	dec h
	or  h
	jr  nz, MOVEN2

SETAD:
;set (CSRAD)
	ld	bc,(CSRX)	;c=X,b=Y y*64
	ld  a,b
	add a,a		; Y*2
	add a,a		; Y*4
	add a,a		; Y*8
	ld  h,0
	ld  l,a
	add hl,hl	; Y*16
	add hl,hl   ; Y*32
	add hl,hl   ; Y*64
	ld  b,0
	add hl,bc
	ld de,VDISP
	add hl, de
	ld	(CSRAD),hl
	ret

CTLCR:				;0dh(CR)
	xor	a
	ld	(CSRX),a
	jr	SETAD

CTLUP:				;0bh(VT)
	ld	hl,CSRY
	ld	a,(hl)
	or	a
	ret	z
	dec	(hl)
	jr	SETAD

CTLRHT:				;09h(HT),0ch(FF),1ch(FS)
	ld	hl,CSRX
	ld	bc,(WIDTH)
	ld	a,(hl)
	inc	a
	cp	c
	jr	c,RHTOK
	inc	hl		;CSRY
	ld	a,(hl)
	dec	b
	cp	b
	ret	nc
	inc	(hl)
	dec	hl		;CSRX
	xor	a
RHTOK:
	ld	(hl),a
	jr	SETAD
	
CTLLFT:				;08h(BS),1dh(GS)
	ld	hl,CSRX
	ld	a,(hl)
	or	a
	jr	nz,LFTOK
	inc	hl		;CSRY
	ld	a,(hl)
	or	a
	ret	z
	dec	(hl)
	dec	hl		;CSRX
	ld	a,(WIDTH)
LFTOK:
	dec	a
	ld	(hl),a
	jr	SETAD

CTLHOM:				;1eh(RS)
	ld	hl,0
	ld	(CSRX),hl
	jr	SETAD

CTLCLS:				;1ah(SUB)
	ld	hl,VDISP
	ld	de,VDISP+1
	ld	bc,64*24-1
	ld	(hl)," "
	ldir

	ld bc, 32 * 192
	ld l, 0
LOOP1:
	out (c), l
	DEC BC
	LD A, B
	OR C
	JR NZ, LOOP1
	
	jr	CTLHOM		;?

CTLESC:				;1bh(ESC)
	ld	a,1
	ld	(ESCMOD),a
CTLNUL:				;00h(NUL),etc.
	ret

ESC:
	dec	a		;a=(ESCMOD)
	jr	z,ESC1
	dec	a
	jr	z,ESC2

ESC3:
	xor	a
	ld	(ESCMOD),a
	ld	a,c
	sub	20h
	ld	bc,(WIDTH)
	cp	c
	jr	c,XOK
	ld	a,c
	dec	a
XOK:
	ld	(CSRX),a
	ld	a,(ESCEQY)
	sub	20h
	cp	b
	jr	c,YOK
	ld	a,b
	dec	a
YOK:
	ld	(CSRY),a
	jp	SETAD

ESC2:
	ld	a,3
	ld	(ESCMOD),a
	ld	a,c
	ld	(ESCEQY),a
	ret

ESC1:
	ld	(ESCMOD),a	;a=0
	ld	a,c
	cp	"="
	jr	z,ESCEQ
	cp	"T"
	jr	z,ESCT
	cp	"Y"
	jr	z,ESCY
	cp	"*"
	jr	z,CTLCLS
	ret

ESCEQ:				;set cursor (ESC=YX)
	ld	a,2
	ld	(ESCMOD),a
	ret

ESCT:				;clear to line end
	ld	hl,(CSRX)
	push	hl
	call	ESCTLP
	pop	hl
	ld	(CSRX),hl
	jp	SETAD

ESCTLP:
	ld	a," "
	call	COPYCHR
	jr	nz,ESCTLP
	ret

ESCY:				;clear to screen end
	ld	hl,(CSRX)
	push	hl
ESCYLP:
	call	ESCTLP
	ld	hl,CSRX
	ld	(hl),0
	inc	hl
	inc	(hl)
	ld	a,(hl)
	cp	24
	jr	c,ESCYLP
	pop	hl
	ld	(CSRX),hl
	ret

CALCAD:			; af,bc,hl
;calculate VRAM address
	xor a
	ld  l,a
	ld	a,(CSRY)
	ld	h,a		;Y*256
	ld	b,VRAM/256
	ld	a,(CSRX)
	rra			;X/2
	ld	c,a
	add	hl,bc		;VRAM+Y*256+X/2
	ret

COPYCHR:	;return z-flag=1 if cursor reach line end
	ld	hl,(CSRAD)
	cp	(hl)
	jr	z,INCCSR
	ld	(hl),a
	add	a,a
	ld	h,0
	ld	l,a
	add	hl,hl
	ld	de,FONT-80h
	add	hl,de
	ex	de,hl
;	call	CALCAD		;de no change
	xor a
	ld  a,(CSRY)
	ld  h, a
	ld  a,(CSRX)
	rra
	ld  l,a
	ld	b,4
	jr	c,XODD

XEVEN:
;x=0,2,4,...
	push bc
	ld  b,h
	ld  c,l
	in  a, (c)
	ld  c, a
	ld	a,(de)		;font
	xor	c
	and	0f0h
	xor	c
	ld  c,l
	out (c), a
	ld  bc, 32
	add hl, bc
	ld  b,h
	ld  c,l
	in  a, (c)
	ld  c, a
	ld	a,(de)		;font
	rlca
	rlca
	rlca
	rlca
	xor	c
	and	0f0h
	xor	 c
	ld  c,l
	out (c), a
	ld  bc, 32
	add hl, bc
	inc de
	pop bc
	djnz	XEVEN
	jr	INCCSR

XODD:
;x=1,3,5,...
	push bc
	ld  b, h
	ld  c, l
	in  a, (c)
	ld  c, a
	ld	a,(de)		;font
	rrca
	rrca
	rrca
	rrca
	xor	c
	and	0fh
	xor	c
	ld  c, l
	out (c), a
	ld  bc, 32
	add hl, bc
	ld  b, h
	ld  c, l
	in  a, (c)
	ld  c, a
	ld	a,(de)		;font
	xor	c
	and	0fh
	xor	c
	ld  c, l
	out (c), a
	ld  bc, 32
	add hl, bc
	inc	de
	pop bc
	djnz	XODD

INCCSR:
	ld	hl,(CSRAD)
	inc	hl
	ld	(CSRAD),hl
	ld	hl,CSRX
	inc	(hl)
	ld	a,(WIDTH)
	cp	(hl)
	ret	
	
FONT:
	db	000h,000h,000h,000h	;sp
	db	044h,044h,040h,040h	;!
	db	0aah,000h,000h,000h	;"
	db	00ah,0eah,0eah,000h	;#
	db	04eh,0ceh,06eh,040h	;$
	db	008h,024h,082h,000h	;%
	db	04ah,0a4h,0a8h,060h	;&
	db	048h,000h,000h,000h	;'
	db	024h,044h,044h,020h	;(
	db	084h,044h,044h,080h	;)
	db	004h,0e4h,0e4h,000h	;*
	db	004h,04eh,044h,000h	;+
	db	000h,000h,044h,080h	;,
	db	000h,00eh,000h,000h	;-
	db	000h,000h,000h,040h	;.
	db	022h,044h,048h,080h	;/

	db	04ah,0aeh,0aah,040h	;0
	db	04ch,044h,044h,0e0h	;1
	db	04ah,0a2h,048h,0e0h	;2
	db	04ah,024h,02ah,040h	;3
	db	026h,0aah,0e2h,020h	;4
	db	0e8h,08ch,022h,0c0h	;5
	db	04ah,08ch,0aah,040h	;6
	db	0eah,022h,044h,040h	;7
	db	04ah,0a4h,0aah,040h	;8
	db	04ah,0a6h,02ah,040h	;9
	db	004h,040h,044h,000h	;:
	db	004h,040h,044h,080h	;;
	db	002h,048h,042h,000h	;<
	db	000h,0e0h,0e0h,000h	;=
	db	008h,042h,048h,000h	;>
	db	04ah,024h,040h,040h	;?

	db	04ah,022h,06ah,040h	;@
	db	04ah,0aeh,0aah,0a0h	;A
	db	0cah,0ach,0aah,0c0h	;B
	db	04ah,088h,08ah,040h	;C
	db	0cah,0aah,0aah,0c0h	;D
	db	0e8h,08eh,088h,0e0h	;E
	db	0e8h,08eh,088h,080h	;F
	db	068h,08eh,0aah,040h	;G
	db	0aah,0aeh,0aah,0a0h	;H
	db	0e4h,044h,044h,0e0h	;I
	db	0e4h,044h,044h,080h	;J
	db	0aah,0cch,0aah,0a0h	;K
	db	088h,088h,088h,0e0h	;L
	db	0aeh,0eeh,0aah,0a0h	;M
	db	0aeh,0eeh,0eeh,0a0h	;N
	db	04ah,0aah,0aah,040h	;O

	db	0cah,0ach,088h,080h	;P
	db	04ah,0aah,0a4h,020h	;Q
	db	0cah,0ach,0aah,0a0h	;R
	db	04ah,084h,02ah,040h	;S
	db	0e4h,044h,044h,040h	;T
	db	0aah,0aah,0aah,0e0h	;U
	db	0aah,0aah,0aah,040h	;V
	db	0aah,0eeh,0eeh,0a0h	;W
	db	0aah,0a4h,0aah,0a0h	;X
	db	0aah,0a4h,044h,040h	;Y
	db	0e2h,0a4h,0a8h,0e0h	;Z
	db	064h,044h,044h,060h	;[
	db	0aah,04eh,04eh,040h	;\
	db	0c4h,044h,044h,0c0h	;]
	db	04ah,000h,000h,000h	;^
	db	000h,000h,000h,0e0h	;_

	db	042h,000h,000h,000h	;`
	db	000h,0c2h,0eah,0e0h	;a
	db	088h,08ch,0aah,0c0h	;b
	db	000h,068h,088h,060h	;c
	db	022h,026h,0aah,060h	;d
	db	000h,04ah,0e8h,060h	;e
	db	024h,04eh,044h,040h	;f
	db	000h,06ah,062h,0c0h	;g
	db	088h,08ch,0aah,0a0h	;h
	db	040h,044h,044h,040h	;i
	db	020h,022h,02ah,040h	;j
	db	088h,0ach,0cah,0a0h	;k
	db	044h,044h,044h,060h	;l
	db	000h,0eeh,0eeh,0a0h	;m
	db	000h,0cah,0aah,0a0h	;n
	db	000h,04ah,0aah,040h	;o

	db	000h,0cah,0c8h,080h	;p
	db	000h,06ah,062h,020h	;q
	db	000h,0ach,088h,080h	;r
	db	000h,068h,042h,0c0h	;s
	db	004h,0e4h,044h,060h	;t
	db	000h,0aah,0aah,060h	;u
	db	000h,0aah,0aah,040h	;v
	db	000h,0aah,0eeh,0a0h	;w
	db	000h,0aah,04ah,0a0h	;x
	db	000h,0aah,062h,0c0h	;y
	db	000h,0e2h,048h,0e0h	;z
	db	024h,048h,044h,020h	;{
	db	044h,040h,044h,040h	;|
	db	084h,042h,044h,080h	;}
	db	000h,02eh,080h,000h	;~
	db	000h,000h,000h,000h	;none

	
;
;
; Workarea for SPC-1000
;
CAPS:	db	0		;caps lock, non-zero=on
ESCMOD:	db	0		;waiting for ESC character
ESCEQY:	db	0		;
TIME:	db	0		;2ms timer

CSRX:	db	0		;cursor x position on virtual display
CSRY:	db	0		;cursor y positin on virtual display
CSRAD:	dw	VDISP		;cursor address on virtual display
CSRATT:	dw	0		;cursor attribute address on real display
				; used and changed by only CONIN and INTGAM
CSROUT:	db	0		;cursor in/out of real display (non-zero=out)

                   	;BIOS work area
WIDTH:	db	64		;virtual display size=80x25
HEIGHT:	db	24		;
	ds	1
VDAREA:	db	0		;display area number in virtual display

VRAM	equ	0h		;VRAM plane1
;ATTR	equ	0e000h		;VRAM plane2
	


GMODEF: DEFB	0
CURX:   DEFB    0                       ;11ed  00            2867   2874 ; cursor X
CURY:   DEFB    0                       ;11ee  00            2868   2875 ; cursor Y
OLDBUF: DEFS    10                      ;11ef                2869   2876 ; keyboard matrix old buffer
NEWBUF: DEFS    9                       ;11f9                2870   2877 ; keyboard matrix new buffer
NEWMOD: DEFB    0FFh                    ;1202  ff            2871   2878 ; keyboard matrix mode flag
                                        ;                    2872   2879 ;
LOCKMD: DEFB    0                       ;1203  00            2873   2880 ; LOCK mode
POINT1: DEFB    0                       ;1204  00            2874   2881 ; key input buffer 
POINT2: DEFB    0                       ;1205  00            2875   2882 ; key input buffer 
                                        ;                    2878   2885 ;
                                        ;                    2879   2886 ;
FUNS 	EQU  8										
FUNBUF: 
	DEFB    4                     
    DEFM    'DIR'                 
    DEFB    00Dh                  
    DEFS    11-FUNS
	
    DEFB    4                     
    DEFM    'ERA '                
    DEFB    0                     
    DEFS    10-FUNS     
	

    DEFB    4                     
    DEFM    'REN '                
    DEFB    0                     
    DEFS    10-FUNS                    

    DEFB    5                     
    DEFM    'SAVE '               
    DEFB    0                     
    DEFS    9-FUNS                    

    DEFB    5                     
    DEFM    'TYPE '               
    DEFB    0                     
    DEFS    9-FUNS                    

    DEFB    4                     
    DEFM    'PIP '                
    DEFB    0                  
    DEFS    10-FUNS                    

    DEFB    4               
    DEFM    'STAT '                
    DEFB    0                  
    DEFS    10-FUNS                    

    DEFB    3                     
    DEFM    'ED '               
    DEFB    0                     
    DEFS    11-FUNS                     

    DEFB    4                     
    DEFM    'ASM '                
    DEFB    0                     
    DEFS    10-FUNS                    

    DEFB    4                     
    DEFM    'DDT '                
    DEFB    0                  
    DEFS    10-FUNS                    
                                       ;
KEYDT1: DEFW    0F500h                 ;8009  No Shift
    DEFM    '-0;'                      ;'		;'
    DEFW    06F6Ch                     ;LO
    DEFM    '9'                        ;9
    DEFW    0F400h                     ;8008
    DEFB    01Fh                       
    DEFM    ':/'                       ;/
    DEFW    0696Bh                     ;KI
    DEFM    '8'                        
    DEFW    0F300h                     ;8007
    DEFW    0701Eh                     ;P
    DEFM    '.'                        ;
    DEFW    0756Ah                     ;JU
    DEFM    '7'                        
    DEFW    0F200h                     ;8006
    DEFW    07840h                     ;X
    DEFM    ','                        
    DEFW    07968h                     ;HV
    DEFM    '6'                        
    DEFW    0F100h                     ;8005
    DEFW    05D1Dh                     
    DEFW    0676Dh                     ;MG
    DEFW    03574h                     ;T5
    DEFW    00000h                     ;8004
    DEFW    07C1Ch                     
    DEFW    0666Eh                     ;NF
    DEFW    03472h                     ;R4
    DEFW    00008h                     ;DEL ;8003
    DEFW    05B1Bh                     
    DEFW    06462h                     ;BD
    DEFW    03365h                     ;E3
    DEFW    00016h                     ;LOCK ;8002
    DEFW    05D7Ah                     ;Z^
    DEFW    07376h                     ;VS
    DEFW    03277h                     ;W2
    DEFW    00B5Eh                     ;HOME ;8001
    DEFW    00D20h                     
    DEFW    06163h                     ;CA
    DEFW    03171h                     ;Q1
                                       ;
                                       ;
                                       ;
KEYDT2: DEFW    0FA00h                 ; Shift
    DEFM    '=0+LO)'                   
    DEFW    0F900h                     
    DEFB    01Fh                       
    DEFM    '*?KI('                    
    DEFW    0F800h                     
    DEFB    01Eh                       
    DEFM    'P>JU'                     
    DEFB    027h                       
    DEFW    0F700h                     
    DEFB    060h                       
    DEFM    'X<HY&'                    
    DEFW    0F600h                     
    DEFW    07D1Dh                     
    DEFM    'MGT%'                     
    DEFW    00000h                     
    DEFW    07F1Ch                     
    DEFM    'NFR$'                     
    DEFW    00012h                     ;INST
    DEFW    07B1Bh                     
    DEFM    'BDE#'                     
    DEFW    00017h                     ;LOCK
    DEFM    'Z'                        
    DEFB    07Dh                       
    DEFM    'VSW"'                     
    DEFW    00C7Eh                     ;CLR
    DEFW    00D20h                     
    DEFM    'CAQ!'                     	
	REF			;of BIOS

;
;	the remainder of the CBIOS is reserved uninitialized
;	data area, and does not need to be a part of the
;	system memory image (the space must be available,
;	however, between "begdat" and "enddat").
;
;	scratch ram area for BDOS use
;
BEGDAT	EQU	$		;beginning of data area
DIRBF:	DEFS	128		;scratch directory area
ALL00:	DEFS	31		;allocation vector 0
ALL01:	DEFS	31		;allocation vector 1
ALL02:	DEFS	31		;allocation vector 2
ALL03:	DEFS	31		;allocation vector 3
ALLHD1:	DEFS	255		;allocation vector harddisk 1
ALLHD2:	DEFS	255		;allocation vector harddisk 2
CHK00:	DEFS	16		;check vector 0
CHK01:	DEFS	16		;check vector 1
CHK02:	DEFS	16		;check vector 2
CHK03:	DEFS	16		;check vector 3
CHKHD1:	DEFS	0		;check vector harddisk 1
CHKHD2:	DEFS	0		;check vector harddisk 2
;KEYBUF: DEFS    40h
INKYBF: DEFS    040h                    ;1206                2876   2883 ; key input buffer
TABBUF: DEFS    32                      ;1246                2877   2884 ; TAB flag
VDISP:  defs 64*24  ;virtual display address
DISKDATA: DEFS	256
;
ENDDAT	EQU	$		;end of data area
DATSIZ	EQU	$-BEGDAT	;size of data area
;

