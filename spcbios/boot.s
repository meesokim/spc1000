;==============================================================================
;   Floppy booting code for SPC-1000
;
;           boot.s
;                                   By meeso.kim
;==============================================================================

    .module boot
    .area   _CODE
    .area  _HEADER  (ABS)
    .org    CPM_BOOT
    
MSIZE   =   58
BIAS    =   (MSIZE-20)*1024
CCP     =   0x3400+BIAS
BIOS    =   CCP+0x1600
BIOSL   =   0x0300      ;length of the bios
BDOS    =   CCP+0x806   ;base of bdos

;BOOT   =   BIOS
SIZE    =   BIOS+BIOSL-CCP  ;size of cp/m system
SECTS   =   SIZE/128    ;# of sectors to load
NSECTS  =   (BIOS-CCP)/128
DSKIX   =   0x0fa00-33
CLS     =   0x1b42
CLR02   =   0x0ad5
GSAVES  =   0x1bea
CDISK   =   0x0004      ;current disk number 0=A,...,15=P
IOBYTE  =   0x0003      ;intel i/o byte
TICONT  =   0x0036

SDWRITE     = 1
SDREAD      = 2
SDSEND      = 3
SDCOPY      = 4
SDFORMAT    = 5
            
CPM_BOOT = 0xcb00   

start::
;   .ascii "SYS"
    ld  ix, #DSKIX
    ld  sp, ix
	call CLR02
    call GSAVES
    ld  bc, #0x002
    ld  h,  #15
    ld  de, #CCP
    call _sd_load
    ld  bc, #0x101
    ld  h,  #8
    ld  de, #CCP+0xf00
    call _sd_load
    ld  bc, #0x109
    ld  h,  #8
    ld  de, #BIOS
    call _sd_load
    ld  bc, #0x201
    ld  h,  #7
    ld  de, #BIOS+0x800
    call _sd_load
	im 1
	ei
	call #0x56
    jp BIOS
    
_sd_load:
    push hl
    push de
    push bc
    ld  d, #SDREAD
    call sendcmd
    ld  d, h
    call senddata
    ld  d, #0
    call senddata
    pop hl
    ld  d, h
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
    LD  B,#0xC0             
    LD  C,#0x02             
    LD  A,#0x80             
    OUT (C),A           
senddata:   
    LD  B,#0xC0             
    LD  C,#0x02             
CHKRFD1:    
    IN  A,(C)           
    AND #0x02           
    JR  Z,CHKRFD1       
    LD  C,#0x02             
    XOR A               
    OUT (C),A           
    LD  C,#0x00             
    OUT (C),D           
    LD  C,#0x02             
    LD  A,#0x10             
    OUT (C),A           
    LD  C,#0x02         
CHKDAC2:    
    IN  A,(C)   
    AND #0x04           
    JR  Z,CHKDAC2       
    LD  C,#0x02         
    XOR A             
    OUT (C),A           
    LD  C,#0x02         
CHKDAC3:    
    IN  A,(C)          
    AND #0x04           
    JR  NZ,CHKDAC3      
    RET               
    
recvdata:
    PUSH    BC           
    LD  C,#0x02             
    LD  B,#0xC0             
    LD  A,#0x20             
    OUT (C),A           
    LD  C,#0x02             
CHKDAV0:    
    IN  A,(C)           
    AND #0x01           
    JR  Z,CHKDAV0       
    LD  C,#0x02         
    XOR A               
    OUT (C),A           
    LD  C,#0x01             
    IN  D,(C)           
    LD  C,#0x02             
    LD  A,#0x40             
    OUT (C),A         
    LD  C,#0x02             
CHKDAV1:    
    IN  A,(C)           
    AND #0x01           
    JR  NZ,CHKDAV1      
    LD  C,#0x02         
    XOR A               
    OUT (C),A           
    POP BC              
    RET           

    .org BIOS
    
    JP BOOT     ;cold start
WBOOTE: JP  WBOOT       ;warm start
    JP CONST        ;console status
    JP CONIN        ;console character in
    JP CONOUT       ;console character out
    JP LIST     ;list character out
    JP PUNCHS       ;punch character out
    JP READER       ;reader character in
    JP HOME     ;move head to home position
    JP SELDSK       ;select disk
    JP SETTRK       ;set track number
    JP SETSEC       ;set sector number
    JP SETDMA       ;set dma address
    JP READ     ;read disk
    JP WRITE        ;write disk
    JP LISTST       ;return list status
    JP SECTRAN      ;sector translate

DPBASE: 
    .dw 0,0
    .dw 0,0
    .dw DIRBF,DPBLK
    .dw CHK00,ALL00
;   disk parameter header for disk 01
    .dw 0,0
    .dw 0,0
    .dw DIRBF,DPBLK
    .dw CHK01,ALL01
;
;   sector translate vector for the IBM 8" SD disks
;
TRANS:  
    .db 1,7,13,19   ;sectors 1,2,3,4
    .db 25,5,11,17  ;sectors 5,6,7,8
    .db 23,3,9,15   ;sectors 9,10,11,12
    .db 21,2,8,14   ;sectors 13,14,15,16
    .db 20,26,6,12  ;sectors 17,18,19,20
    .db 18,24,4,10  ;sectors 21,22,23,24
    .db 16,22       ;sectors 25,26
;
;   disk parameter block, SD725 (from TF-20)
;
DPBLK:  
    .dw 40      ;sectors per track
    .db 4       ;block shift factor
    .db 15      ;block mask
    .db 1       ;extent mask
    .dw 153     ;disk size-1
    .dw 127     ;directory max
    .db 0xc0    ;alloc 0
    .db 0       ;alloc 1
    .dw 32      ;check size
    .dw 3       ;track offset

;   messages
;
SIGNON: 
    .ascii  '58K CP/M Version 2.2'
    .db 13,10
    .ascii  '(for SPC-1000 with SD-725)'
    .db 13,10,0
;
LDERR:  
    .ascii  'BIOS: error booting'
    .db 13,10,0 

DIRBF:  .ds 128     ;scratch directory area
ALL00:  .ds 31      ;allocation vector 0
ALL01:  .ds 31      ;allocation vector 1
ALL02:  .ds 31      ;allocation vector 2
ALL03:  .ds 31      ;allocation vector 3
ALLHD1: .ds 255     ;allocation vector harddisk 1
ALLHD2: .ds 255     ;allocation vector harddisk 2
CHK00:  .ds 16      ;check vector 0
CHK01:  .ds 16      ;check vector 1
CHK02:  .ds 16      ;check vector 2
CHK03:  .ds 16      ;check vector 3
CHKHD1: .ds 0       ;check vector harddisk 1
CHKHD2: .ds 0       ;check vector harddisk 2


FDDN:   .db 1
FDDD:   .db 0
FDDT:   .db 0
FDDS:   .db 0
DATADDR:.dw 0
CHGFLG: .db 1
PRESEC: .db 0xff    

PRTMSG: LD  A,(HL)
    OR  A
    RET Z
    LD  C,A
    CALL    CONOUT
    INC HL
    JP  PRTMSG
;
;   individual subroutines to perform each function
;   simplest case is to just perform parameter initialization
;
BOOT:   
    LD  SP,#0x80        ;use space below buffer for stack
    XOR A
    LD  BC, #0x6000
    OUT (C), A
	LD  B,#0
	CALL SCRNBC	
    LD  HL,#SIGNON  ;print message
    CALL    PRTMSG
    XOR A       ;zero in the accum
    LD  (#IOBYTE),A ;clear the iobyte
    LD  (#CDISK),A  ;select disk zero
    JP  GOCPM       ;initialize and go to cp/m  
WBOOT:      ;warm start
    LD  SP,#0x80        ;use space below buffer for stack
    LD  C,#0        ;select disk 0
    CALL    SELDSK
    CALL    HOME        ;go to track 00
;
    LD  B,#NSECTS   ;b counts # of sectors to load
    LD  C,#0        ;c has the current track number
    LD  D,#4        ;d has the next sector to read
;   note that we begin by reading track 0, sector 2 since sector 1
;   contains the cold start loader, which is skipped in a warm start
    LD  HL,#CCP     ;base of cp/m (initial load point)
LOAD1:              ;load one more sector
    PUSH    BC      ;save sector count, current track
    PUSH    DE      ;save next sector to read
    PUSH    HL      ;save dma address
    LD  C,D     ;get sector address to register c
    CALL    SETSEC      ;set sector address from register c
    POP BC      ;recall dma address to b,c
    PUSH    BC      ;replace on stack for later recall
    CALL    SETDMA      ;set dma address from b,c
;   drive set to 0, track set, sector set, dma address set
    CALL    READ
    OR  A       ;any errors?
    JP  Z,LOAD2     ;no, continue
    LD  HL,#LDERR   ;error, print message
    CALL    PRTMSG
    DI          ;and halt the machine
    HALT
;   no error, move to next sector
LOAD2:  POP HL      ;recall dma address
    LD  DE,#128     ;dma=dma+128
    ADD HL,DE       ;new dma address is in h,l
    POP DE      ;recall sector address
    POP BC      ;recall number of sectors remaining,
                ;and current trk
    DEC B       ;sectors=sectors-1
    JP  Z,GOCPM     ;transfer to cp/m if all have been loaded
;   more sectors remain to load, check for track change
    INC D
    LD  A,D     ;sector=27?, if so, change tracks
    CP  #31
    JP  C,LOAD1     ;carry generated if sector<27
;   end of current track, go to next track
    LD  D,#1        ;begin with first sector of next track
    INC C       ;track=track+1
;   save register state, and change tracks
    CALL    SETTRK      ;track address set from register c
    JP  LOAD1       ;for another sector
;   end of load operation, set parameters and go to cp/m
GOCPM:
    LD  A,#0x0C3        ;c3 is a jmp instruction
    LD  (#0),A      ;for jmp to wboot
    LD  HL,#WBOOTE  ;wboot entry point
    LD  (#1),HL     ;set address field for jmp at 0
;
    LD  (#5),A      ;for jmp to bdos
    LD  HL,#BDOS        ;bdos entry point
    LD  (#6),HL     ;address field of jump at 5 to bdos
;
    LD  BC,#0x80        ;default dma address is 80h
    CALL    SETDMA
;
    LD  A,(#CDISK)  ;get current disk number
    LD  C,A     ;send to the ccp
    JP  CCP     ;go to cp/m for further processing  
CONST:      ;console status
    LD A, (#KEYBUF)
    OR A
    RET NZ
    LD A, #0xff
    RET
CONIN:      ;console character in
;   IN  A,(CONDAT)  ;get character from console
    CALL ASCGET1        
    RET
CONOUT:     ;console character out
    LD  A,C     ;get to accumulator
ASCPRT:
    PUSH HL
    CALL ACCPRT1
    POP  HL
    RET
LIST:       ;list character out
    LD  A,C     ;character to register a
    
PUNCHS:     ;punch character out
    LD  A,C     ;character to register a
;   OUT (AUXDAT),A
    RET
READER:     ;reader character in
    RET
HOME:       ;move head to home position
    LD  C,#0        ;select track 0
    JP  SETTRK      ;we will move to 00 on first read/write
SELFD:
    LD  C,A
    LD  A,(#FDDD)
    CP  C
    JR  Z,SELFD$ 
    LD HL,#CHGFLG
    SET #2,(HL)
SELFD$: 
    LD H,#0
    LD (#FDDD),A
    LD  L,A     ;L=disk number 0,1,2,3
    ADD HL,HL       ;*2
    ADD HL,HL       ;*4
    ADD HL,HL       ;*8
    ADD HL,HL       ;*16 (size of each header)
    LD  DE,#DPBASE
    ADD HL,DE       ;HL=.dpbase(diskno*16)
    RET
SELDSK:     ;select disk
    LD  HL,#0   ;error return code
    LD  A,C
    CP  #2      ;FD drive 0-1?
    JR  C,#SELFD        ;go
    RET 
SETTRK:     ;set track number
    LD A,(#FDDT)
    CP C
    RET Z
    LD HL,#CHGFLG
    SET 1,(HL)
    LD  A,C
    LD (#FDDT),A
    RET
SETSEC:     ;set sector number
    DEC C
    LD B,C
    XOR A
    LD A,(#FDDS)
    RR A
    OR A
    RR C
    CP C
    JR Z, SETSEC0
    LD HL,#CHGFLG
    SET #0,(HL)
SETSEC0:    
    LD  A,B
    LD (#FDDS),A
    RET
SETDMA:     ;set dma address
    LD (#DATADDR), BC
    RET
LISTST:     ;return list status
    RET
SECTRAN:    ;sector translate
    LD  A,D     ;do we have a translation table?
    OR  E
    JP  NZ,SECT1    ;yes, translate
    LD  L,C     ;no, return untranslated
    LD  H,B     ;in HL
    INC L       ;sector no. start with 1
    RET NZ
    INC H
    RET
SECT1:  EX  DE,HL       ;HL=.trans
    ADD HL,BC       ;HL=.trans(sector)
    LD  L,(HL)      ;L = trans(sector)
    LD  H,#0        ;HL= trans(sector)
    RET         ;with data in HL

READ:       ;read disk
    LD  D, #SDREAD                       ;e3aa  16 0e          355    369 
    CALL    sendcmd                     ;e3ac  cd 01 e4       356    370 
    LD  HL,#FDDN                        ;e3af  21 67 e4       357    371 
    LD  D,(HL)                          ;e3b2  56             358    372 
    CALL    senddata                     ;e3b3  cd 09 e4       359    373 
    INC HL                              ;e3b6  23             360    374 
    LD  D,(HL)                          ;e3b7  56             361    375 
    CALL    senddata                     ;e3b8  cd 09 e4       362    376 
    INC HL                              ;e3bb  23             363    377 
    LD  D,(HL)                          ;e3bc  56             364    378 
    CALL    senddata                     ;e3bd  cd 09 e4       365    379 
    INC HL                              ;e3c0  23             366    380 
    LD  D,(HL)                          ;e3c1  56             367    381 
    CALL    senddata                     ;e3c3  cd 09 e4       369    383 
    ld d, #SDSEND
    call sendcmd
    ld hl, (#DATADDR)
    ld bc, #0x100
RDLOOP:
    call recvdata
    ld (hl), d
    inc hl
    dec bc
    ld  a, b
    or  c
    jr nz, RDLOOP
    ret
    
WRITE:      ;write disk 
    LD  D, #SDWRITE                       ;e3aa  16 0e          355    369 
    CALL    sendcmd                     ;e3ac  cd 01 e4       356    370 
    LD  HL,#FDDN                        ;e3af  21 67 e4       357    371 
    LD  D,(HL)                          ;e3b2  56             358    372 
    CALL    senddata                     ;e3b3  cd 09 e4       359    373 
    INC HL                              ;e3b6  23             360    374 
    LD  D,(HL)                          ;e3b7  56             361    375 
    CALL    senddata                     ;e3b8  cd 09 e4       362    376 
    INC HL                              ;e3bb  23             363    377 
    LD  D,(HL)                          ;e3bc  56             364    378 
    CALL    senddata                     ;e3bd  cd 09 e4       365    379 
    INC HL                              ;e3c0  23             366    380 
    LD  D,(HL)                          ;e3c1  56             367    381 
    CALL    senddata                     ;e3c3  cd 09 e4       369    383 
    ld d, #SDSEND
    call sendcmd
    ld hl, (#DATADDR)
    ld bc, #0x100
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
    ret 
;
; KEY GET
BRKEY:  LD  A,#0x80
    IN  A,(0)
    AND #0x12
    RET 
ASCGET1: 
    PUSH    BC
    PUSH    DE
    PUSH    HL
    LD  HL,(POINT1)
    LD  A,L
    CP  H
    JR  Z,BUFSET
BUFCEK: INC A
    AND #0x3F
    LD  (POINT1),A
    LD  L,A
    LD  H,#0
    LD  DE,#INKYBF
    ADD HL,DE
    LD  A,(HL)
GETRET: POP HL
    POP DE
    POP BC
    RET
;
BUFSET: .db 0x3E        ; LD A,
FLASHF: .db 0x01        ;
    OR  A
    JR  NZ,FLGET
    CALL    KEYSEN
    CALL    BRKEY
    JR  NZ,KEYGT
BRKGTR: CALL    NEWFLG
    LD  A,#3
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
    LD  A,#0x80
    IN  A,(0)
    AND #0x12
    JR  Z,BRKGTR
    LD  A,C
    OR  A
    JR  Z,NEWKYS
REPETS: CALL    NEWFLG
    .db 0x21        ; LD HL,(KEYBFA)
KEYBFA: .dw 0xFFFE      ;
    .db 0x3E        ; LD A,(KEYBFD)
KEYBFD: .db 0
    LD  (HL),A
    LD  HL,#30
    JR  KEYST2
NEWKYS: CALL    NEWFLG
    .db 0x21        ; LD HL,300
REPTSW: .dw 200     ; auto repeat key duration
KEYST2: LD  (REPETF),HL
    LD  HL,#NEWBUF
    LD  D,#0
    LD  C,#9
SETLP1: LD  B,#8
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
    ADD A,#8
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
FLASHS: CALL    ADRCAL
    SET 3,B
    LD  (FLASHA),BC
    IN  A,(C)
    LD  (FLASHD),A
    OR  #1
    OUT (C),A
    LD  (FLASHO),A
    LD  A,(TICONT)
    LD  (FLASHC+1),A    ;★"+1"이 빠져있었음.
    RET         ;
;
;
FLASHG: .db 1
FLASHA: .dw 0
    .db 0x3E        ; LD A,
FLASHD: .db 0       ;
    AND #0xFE
    LD  E,A
    LD  A,(TICONT)
FLASHC: SUB #0          ; cursor blink time
    AND #0x10
    LD  A,#1
    JR  Z,L0D28
    XOR A
L0D28:  OR  E
    .db 0xFE        ;CP
FLASHO: .db 0
    RET Z
    OUT (C),A
    LD  (FLASHO),A
    RET
;
FLASHE: LD  BC,(FLASHA)
    LD  A,(FLASHD)
    OUT (C),A
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
    LD  A,#0
    JR  NZ,FLSHRT
    CALL    SAMECK
    JR  NZ,FLGTLP
    .db 0x21        ; LD HL,
REPETF: .dw 0       ;
    DEC HL      ;+
    LD  (#REPETF),HL
    LD  A,H
    OR  L
    JR  NZ,FLGTLP
    LD  A,#1
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
KEYSN0: LD  B,#4
KEYSN1: CALL    KEYSN2
    BIT 1,E
    JR  NZ,KEYSN0
    DJNZ    KEYSN1
    LD  A,E
    AND #1
    RET
;
KEYSN2: LD  HL,#NEWBUF
    PUSH    BC
    LD  BC,#0x8009  ;KEYIO
    LD  DE,#0x900
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
    AND #0x12
    JR  NZ,L0DB9
    SET 0,E
L0DB9:  POP BC
    RET
;
NEWCEK: LD  HL,#OLDBUF
    LD  DE,#NEWBUF
    LD  B,#9
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
    AND #0x10
    RET
;
SAMECK: LD  HL,#OLDBUF
    LD  DE,#NEWBUF
    LD  B,#10
SAMELP: LD  A,(DE)
    XOR (HL)
    RET NZ
    INC DE
    INC HL
    DJNZ    SAMELP
    RET
;
NEWFLG: LD  HL,#NEWBUF
    LD  DE,#OLDBUF
    LD  B,#9
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
    LD  DE,#NEWBUF
    LD  HL,#OLDBUF
    LD  BC,#0x0900
FLGLP2: LD  A,(DE)
    CPL
    OR  A
    JR  Z,FLGLP3
    DEC C
    JR  NZ,FLGLP5
    OR  (HL)
    LD  (HL),A
    LD  A,#0xFF
    LD  (DE),A
FLGLP4: LD  C,#1
FLGLP3: INC DE
    INC HL
    DJNZ    FLGLP2
    RET
FLGLP5: PUSH    BC
    LD  BC,#0x801
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
    LD  B,#0
    LD  HL,#KEYDT1  ;NORMAL DATA
    LD  A,(NEWMOD)
    BIT 1,A
    JR  NZ,L0E4C
    LD  HL,#KEYDT2  ;SHIFT DATA
L0E4C:  ADD HL,BC
    LD  C,(HL)
    LD  B,A
    BIT 2,B
    JR  NZ,CTRLNO
    LD  A,C
    SUB #0x40
    JR  C,NONKEZ
    AND #0x1F
    JR  SETKY
CTRLNO: BIT 6,B
    JR  NZ,SETKY2
    LD  A,C
    SUB #0x40
    JR  C,NONKEY
    AND #0x1F
    ADD A,#0x80
SETKY:  CALL    BFSTSB
NONKEY: JP  KYBFSR
NONKEZ: ADD A,#0x10
    CP  #4
    JR  NC,NONKEY
    LD  (COLORF),A
    JR  NONKEY
;
;
SETKY2: LD  A,C
    CP  #0xF0
    JR  NC,FUNCTN
    LD  A,(#LOCKMD)
    OR  A
    LD  A,C
    JR  Z,SETKY
    CP  #0x41
    JR  C,SETKY
    AND #0x1F
    CP  #0x1B
    LD  A,C
    JR  NC,SETKY
    XOR #0x20
    JR  SETKY
FUNCTN: SUB #0xF1
    LD  L,A
    LD  H,#0
    ADD HL,HL
    ADD HL,HL
    ADD HL,HL
    ADD HL,HL
    LD  BC,#FUNBUF
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
    AND #0x3F
    CP  L
    JR  Z,NONSET
    LD  (POINT2),A
    LD  L,A
    LD  H,#0
    LD  A,C
    LD  BC,#INKYBF
    ADD HL,BC
    LD  (HL),A
NONSET: POP BC
    POP HL
NORET:  RET 
; VIDEO RAM ADDRESS CALCULATION
ADRCAL: LD  HL,(CURX)
ADRCA2: XOR A
    SRL H
    RRA
    SRL H
    RRA
    SRL H
    RRA
    OR  L
PAGE05: LD  BC,#0       ;OFSET
    ADD A,C
    LD  C,A
    LD  A,B
    ADC A,H
    LD  B,A
    RET
ACCPRT1:
    PUSH    AF
    PUSH    BC
    PUSH    DE
    PUSH    HL
    CALL    ACCOUS
    POP HL
    POP DE
    POP BC
    POP AF
CTRLSB: 
    RET
;
ACCOUS: 
ACCOUT: 
    CP  #' '        ;020h
    JP  C,CTRLSB
ACCDIS: PUSH    AF
    LD  A,(GMODEF)  ;GMODEF 2,3 X
    CP  #2
    JR  C,ACCOL1
    CP  #4
    POP BC
    RET C
;   CALL    GACPRT
    JR  ACCOL2
ACCOL1: CALL    ADRCAL
    POP AF
    CALL    BCOUTA 
ACCOL2: LD  HL,(CURX)
    INC L
    LD  A,L
    CP  #0x20       ;다음줄 체크
    JR  C,#CURSET
    INC H
    LD  B,#1
CTRLM0: LD  A,H
    CP  #16
    JR  C,CURSET0
    PUSH    BC      ;16줄(1페이지) 체크
PAGE02: LD  BC,#0x0200  ;CLR CHR
    LD  A,#0            ; 소스는 020h
    LD  D,#0x20
SCRLC1: OUT (C),A
    INC BC
    DEC D
    JR  NZ,SCRLC1
PAGE04: LD  BC,#0x0A00
    LD  A,(COLORF)
    AND #0x03
    LD  D,#0x20
SCRLC2: OUT (C),A
    INC BC
    DEC D
    JR  NZ,SCRLC2
    CALL    SCRLUP
    LD  H,#15
    POP BC
CURSET0: LD A,H
    PUSH    HL
    CALL    ATBHL
    LD  (HL),B
    POP HL
    LD  L,#0
CURSET: LD  (CURX),HL
    RET
;
;
SCRLUP: EXX
    PUSH    BC
    PUSH    DE
    PUSH    HL
    LD  BC,(CHRSTA)
    LD  DE,(CHRLE2)
    CALL    BCLDIR
    LD  BC,(ATRSTA)
    LD  DE,(CHRLE2)
    CALL    BCLDIR
;LINE FLG SCROLL
    LD  DE,(LINEFR)
    LD  H,D
    LD  L,E
    INC HL
PAGE11: LD  BC,#63
    LD  A,(DE)
    LDIR
    LD  (DE),A
    POP HL
    POP DE
    POP BC
    EXX
    RET
;
BCLDIR: PUSH    BC
    EXX
    POP BC
    LD  HL,#SCRLBF
    LD  E,#0x20
BCLDL1: IN  A,(C)
    LD  (HL),A
    INC BC
    INC HL
    DEC E
    JR  NZ,BCLDL1
BCLDL2: IN  A,(C)
    INC BC
    EXX
    OUT (C),A
    INC BC
    DEC DE
    LD  A,D
    OR  E
    EXX
    JR  NZ,BCLDL2
    EXX
    LD  HL,#SCRLBF
    LD  E,#0x20
BCLDL3: LD  A,(HL)
    OUT (C),A
    INC HL
    INC BC
    DEC E
    JR  NZ,BCLDL3
    RET
;
;
;SCROLL DOWN
SCRLDW: EXX
    PUSH    BC
    PUSH    DE
    PUSH    HL
PAGE12: LD  BC,#0x07FF  ;★원래는 DEPRTR
    LD  DE,(CHRLE2)
    CALL    BCLDDR
PAGE14: LD  BC,#0x0FFF
    .db     0x11        ; LD DE,
CHRLE2: .dw 0x07E0
    CALL    BCLDDR
;LINE FLG SCROLL
PAGE16: LD  DE,#LINEF+63
    LD  L,E
    LD  H,D
    DEC HL
PAGE17: LD  BC,#63
    LD  A,(DE)
    LDDR
    LD  (DE),A
    POP HL
    POP DE
    POP BC
    EXX
    RET
;
BCLDDR: PUSH    BC
    EXX
    POP BC
    LD  HL,#SCRLBF+0x1F
    LD  E,#0x20
BCLDD1: IN  A,(C)
    LD  (HL),A
    DEC BC
    DEC HL
    DEC E
    JR  NZ,BCLDD1
BCLDD2: IN  A,(C)
    DEC BC
    EXX
    OUT (C),A
    DEC BC
    DEC DE
    LD  A,D
    OR  E
    EXX
    JR  NZ,BCLDD2
    EXX
    LD  HL,#SCRLBF+0x1F
    LD  E,#0x20
BCLDD3: LD  A,(HL)
    OUT (C),A
    DEC HL
    DEC BC
    DEC E
    JR  NZ,BCLDD3
    RET 
;
; 화면에 문자 출력
BCOUTA: CP  #'a'-1      ;060h
    JR  C,BCOTOK
    CP  #0xE0
    JR  NC,SEMIOT
    AND #0x7F
    OUT (C),A
    SET 3,B
    LD  A,(COLORF)
    AND #3
    OR  #8
    OUT (C),A
    RET
;
BCOTOK: OUT (C),A
    SET 3,B
    LD  A,(COLORF)
    AND #3
    OUT (C),A
    RET
;
SEMIOT: AND #0x0F
    PUSH    DE
    LD  D,A
    LD  A,(COLORF)
    DEC A
    AND #7
    RLCA
    RLCA
    RLCA
    RLCA
    OR  D
    POP DE
    OUT (C),A
    SET 3,B
    LD  A,(COLORF)
    AND #3
    OR  #4
    OUT (C),A
    RET 

;
DELETE: CALL    ADRCAL
    PUSH    BC
    CALL    ECUYST
    LD  H,E
    LD  L,#0
    CALL    ADRCA2
    POP DE
    LD  L,C
    LD  H,B
    OR  A
    SBC HL,DE
    LD  D,#0    ;★구버전,신버전 00H,20H ???
    LD  A,(COLORF)
    AND #3
    LD  E,A
DELLP1: DEC BC
    IN  A,(C)
    OUT (C),D
    LD  D,A
    SET 3,B
    IN  A,(C)
    OUT (C),E
    LD  E,A
    RES 3,B
    DEC HL
    LD  A,H
    OR  L
    JR  NZ,DELLP1
    LD  A,(CURX)
    OR  A
    JR  NZ,DELLP2
    PUSH    DE
    CALL    CUTBHL
    LD  A,(HL)
    POP DE
    OR  A
    RET Z
DELLP2: DEC BC
    OUT (C),D
    SET 3,B
    OUT (C),E
    JP  LEFT

CLR2:   
    .db 1       ;LD BC,(CHRSTA)
CHRSTA: .dw 0x0     ;SEMIGRAPHIC ADR.+
    LD  HL,(CHRLEN)
    LD  D,#0        ;★구버전,신버전 00H,20H ???
CLRLP:  OUT (C),D
    INC BC
    DEC HL
    LD  A,H
    OR  L
    JR  NZ,CLRLP
    .db 1       ;LD BC,(ATRSTA)
ATRSTA: .dw 0x0800
    .db 0x21        ;LD HL,(CHRLEN)
CHRLEN: .db 0x0800
    LD  A,(COLORF)
    AND #3
    LD  D,A
ATRCLS: OUT (C),D
    INC BC
    DEC HL
    LD  A,H
    OR  L
    JR  NZ,ATRCLS
    .db 0x21        ;LD HL,(LINEFR)
LINEFR: .dw LINEF
PAGE23: LD  B,#65
LINCLS: LD  (HL),#0
    INC HL
    DJNZ    LINCLS
SCRHOME:    LD  HL,#0
    JP  CURST2
;
INST:   CALL    ECUYST
    LD  H,E
    LD  L,#0
    CALL    ADRCA2
    DEC BC
    CALL    AINBC
    CP  #0x21
    RET NC
    PUSH    BC
    CALL    ADRCAL
    POP HL
    OR  A
    SBC HL,BC
    LD  D,#0x00     ;★구버전,신버전 00H,20H ???
    LD  A,(COLORF)
    AND #3
    LD  E,A
    INC HL
INSTL1: IN  A,(C)
    OUT (C),D
    LD  D,A
    SET 3,B
    IN  A,(C)
    OUT (C),E
    LD  E,A
    RES 3,B
    INC BC
    DEC HL
    LD  A,H
    OR  L
    JR  NZ,INSTL1
    RET
;
LOCK:   LD  A,#1
LOCKST: LD  (LOCKMD),A
    RET
;
UNLOCK: XOR A
    JR  LOCKST
;
LEFT:   LD  HL,(CURX)
    DEC L
    LD  A,L
    CP  #0xFF
    JR  NZ,CURST2
    LD  L,#0x1F
UPCUR:  DEC H
    LD  A,H
    CP  #0xFF
    JR  NZ,CURST2
    PUSH    HL
    CALL    SCRLDW
    POP HL
    LD  H,#0
    JR  CURST2
;
CTRLM:  CALL    ECUYST
    LD  H,E
    LD  B,#0
    JP  CTRLM0
;
RIGHT:  LD  HL,(CURX)
    INC L
    LD  A,L
    CP  #0x20
    JR  C,CURST2
    LD  L,#0
DOWNLB: INC H
    LD  A,H
    CP  #16
    JR  C,CURST2
    PUSH    HL
    CALL    SCRLUP
    POP HL
    LD  H,#15
CURST2: LD  (CURX),HL
    RET
;
UP: LD  HL,(CURX)
    JR  UPCUR
;
DOWN:   LD  HL,(CURX)
    JR  DOWNLB
;
CTRLG:  LD  A,(BELLFG)
    XOR #1
    LD  (BELLFG),A
    JP  BELL
;
CTRLB:  LD  DE,#0
    LD  HL,#LEFT
    LD  (CTRLBF+1),HL
CTRLB1: CALL    CBFCEK
    RET Z
    JR  C,CTRLB1
CTRLB2: CALL    CBFCEK
    RET Z
    JR  NC,CTRLB2
    JP  RIGHT
;
CTRLF:  LD  DE,#0x0F1F
    LD  HL,#RIGHT
    LD  (CTRLBF+1),HL
CTRLF1: CALL    CBFCEK
    RET Z
    JR  NC,CTRLF1
CTRLF2: CALL    CBFCEK
    RET Z
    JR  C,CTRLF2
    RET
;
CBFCEK: LD  HL,(CURX)
    OR  A
    SBC HL,DE
    RET Z
    PUSH    DE
CTRLBF: CALL    RIGHT
    CALL    ADRCAL
    CALL    AINBC
    POP DE
    CP  #'0'        ;030h
    RET C
    CP  #'9'+1      ;03Ah
    CCF
    RET NC
    CP  #'A'        ;041h
    RET C
    CP  #0x80
    JR  C,L0BEC
    OR  A
    RET
L0BEC:  AND #0x1F
    JR  Z,L0BF4
    CP  #0x1B
    CCF
    RET NC
L0BF4:  AND #1
    RET
;
CTRLY:  LD  D,#0        ;★라벨이 빠져있었음
    JR  CTRLYT
;
CTRLT:  LD  D,#1
CTRLYT: LD  HL,(CURX)
    LD  H,#0
    LD  BC,#TABBUF
    ADD HL,BC
    LD  (HL),D
    RET
;
CTRLI:  CALL    RIGHT
    LD  A,(CURX)
    CP  #31
    RET Z
    LD  L,A
    LD  H,#0
    LD  BC,#TABBUF
    ADD HL,BC
    LD  A,(HL)
    OR  A
    JR  Z,CTRLI
    RET
;
CTRLE:  CALL    ECUYST
    LD  H,E
    LD  L,#0
    JR  CTRLEZ
;
CTRLZ:  CALL    CUTBHL
    LD  A,#16
    SUB E
    LD  B,A
    INC HL
CTRLZ1: LD  (HL),#0
    INC HL
    DJNZ    CTRLZ1
    LD  HL,#0x1000
CTRLEZ: CALL    ADRCA2
    DEC BC
    PUSH    BC
    CALL    ADRCAL
    POP HL
    LD  D,B
    LD  E,C
    OR  A
    SBC HL,DE
    INC HL
    LD  D,#0        ;구버전,신버전
    LD  A,(COLORF)
    AND #0x03
    LD  E,A
CTRLZE: OUT (C),D
    SET 3,B
    OUT (C),E
    RES 3,B
    INC BC
    DEC HL
    LD  A,H
    OR  L
    JR  NZ,CTRLZE
    RET
AINBC:  PUSH    BC      ;INSERT키가 눌리면 그 라인의 TEXT 끝부터 32번
    IN  A,(C)       ;COLUMN 사이에 BLANK가 있는지 확인
    AND #0x7F
    SET 3,B
    IN  C,(C)
    BIT 2,C
    JR  Z,ALPHA
    BIT 3,C
    JR  NZ,SEMI6
    AND #0x0F
    ADD A,#0xE0
    JR  AINOK
ALPHA:  BIT 3,C
    JR  Z,AINOK
    CP  #0x60
    JR  NC,AINOK
    ADD A,#0x80
AINOK:  POP BC
    RET
SEMI6:  POP BC
    XOR A
    RET 
BCUYST: CALL    CUTBHL
    XOR A
LPBNST: CP  (HL)
    RET Z
    DEC HL
    DEC E
    JR  NZ,LPBNST
    RET
;
ECUYST: CALL    CUTBHL
ECUYS2: INC HL
    INC E
    LD  A,E
    CP  #16     ;16라인(1페이지)를 통과했는지 체크
    RET NC
    XOR A
    CP  (HL)
    JR  NZ,ECUYS2
    RET
;
CUTBHL: LD  A,(CURY)
ATBHL:  LD  E,A
    LD  D,#0
    LD  HL,(LINEFR)
    ADD HL,DE       ;CURY+LINEFR(01 OR 00)
    RET
BELL:   
    RET
IO20SB: LD	BC,#0x2000
	OUT	(C),A
	LD	(IO2000),A
	RET	
SCRNBC: LD	A,C		;SCREEN B,C
	CP	#5
	RET	NC
	OR	A
	LD	HL,#SCRN00
	JR	Z,SCRNS0
	LD	A,B
	LD	B,#0
	DEC	C
	LD	HL,#SCRNTB
	RL	C
	ADD	HL,BC
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	EX	DE,HL
	DEC	A
PATCH4:	
	RLCA
	RLCA
	RLCA
	RLCA
SCRNS0: LD	E,A
	LD	A,(IO2000)
	AND	#0xCF
	OR	E
	CALL	IO20SB
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	INC	HL
	EX	DE,HL
	LD	(LINEFR),HL
	EX	DE,HL
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	INC	HL
	EX	DE,HL
	LD	(PAGE16+1),HL
	EX	DE,HL
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	INC	HL
	EX	DE,HL
	LD	(PAGE11+1),HL
	LD	(PAGE17+1),HL
	LD	BC,#2		;★이 2줄 4바이트는
	ADD	HL,BC		;INC HL 두 번 2바이트면 가능하다.
	LD	A,L
	LD	(PAGE23+1),A
	EX	DE,HL
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	INC	HL
	EX	DE,HL
	LD	(CHRSTA),HL
	LD	(PAGE05+1),HL
	PUSH	HL
	SET	3,H
	LD	(ATRSTA),HL
	EX	DE,HL
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	INC	HL
	EX	DE,HL
	LD	(CHRLEN),HL
	LD	BC,#0xFFE0
	ADD	HL,BC
	LD	(CHRLE2),HL
	POP	BC
	ADD	HL,BC
	LD	BC,#0x001F
	ADD	HL,BC
	LD	(PAGE12+1),HL
	SET	3,H
	LD	(PAGE14+1),HL
	EX	DE,HL
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	EX	DE,HL
	LD	(PAGE02+1),HL
	SET	3,H
	LD	(PAGE04+1),HL
	RET
;
;
;
SCRNTB: .dw	SCRN01
	.dw	SCRN02
	.dw	SCRN03
	.dw	SCRN04
;
SCRN00: .dw	LINEF		;LINEFR
	.dw	LINEF+63	 ;PAGE16
	.dw	63		 ;PAGE11
	.dw	0x0000		 ;CHRSTA
	.dw	0x0800		 ;CHRLEN
	.dw	0x0200		 ;PAGE02
;
SCRN01: .dw	LINEF		;LINEFR
	.dw	LINEF+15	 ;PAGE16
	.dw	15		 ;PAGE11
	.dw	0x0000		 ;CHRSTA
	.dw	0x0200		 ;CHRLEN
	.dw	0x0000		 ;PAGE02
;
SCRN02: .dw	LINEF+16	;LINEFR
	.dw	LINEF+31	 ;PAGE16
	.dw	15		 ;PAGE11
	.dw	0x0200		 ;CHRSTA
	.dw	0x0200		 ;CHRLEN
	.dw	0x0200		 ;PAGE02
;
SCRN03: .dw	LINEF+32	;LINEFR
	.dw	LINEF+47	 ;PAGE16
	.dw	015		 ;PAGE11
	.dw	0x0400		 ;CHRSTA
	.dw	0x0200		 ;CHRLEN
	.dw	0x0400		 ;PAGE02
;
SCRN04: .dw	LINEF+48	;LINEFR
	.dw	LINEF+63	 ;PAGE16
	.dw	015		 ;PAGE11
	.dw	0x0600		 ;CHRSTA
	.dw	0x0200		 ;CHRLEN
	.dw	0x0600		 ;PAGE02	

CURX:   .db 0       ; cursor X
CURY:   .db 0       ; cursor Y
OLDBUF: .ds 10      ; keyboard matrix old buffer
NEWBUF: .ds 9       ; keyboard matrix new buffer
NEWMOD: .db 0xFF    ; keyboard matrix mode flag
;
IO2000: .db	0		; 2000H I/O data
LOCKMD: .db 0       ; LOCK mode
POINT1: .db 0       ; key input buffer 
POINT2: .db 0       ; key input buffer 
INKYBF: .ds 0x40    ; key input buffer
TABBUF: .ds 32      ; TAB flag
KEYBUF: .ds 0x40    
;INKYBF: .ds 0x40    ; key input buffer
;TABBUF: .ds 32      ; TAB flag
GMODEF: .db 0
COLORF: .db 0       ; Color Flag
GCOLOR: .db 0       ; Graphic Color
SCRLBF: .ds 0x20    ; scroll buffer
LINEF:  .ds 65      ; line return flag
BELLFG: .db 0       ;CTRL-G

    
FUNBUF: 
    .db 4       ; function definition
    .ascii  "DIR"
    .db 0x0D
    .ds 11
    
    .db 4
    .ascii  'ERA '
    .db 0
    .ds 11

    .db 4
    .ascii  'REN '
    .db 0
    .ds 11

    .db 5
    .ascii  'SAVE '
    .ds 10

    .db 5
    .ascii  'TYPE '
    .ds 10

    .db 4
    .ascii  'PIP '
    .ds 11

    .db 5
    .ascii  'STAT '
    .ds 11

    .db 5
    .ascii  'ED '
    .ds 9

    .db 4
    .ascii  'ASM '
    .ds 11

    .db 4
    .ascii  'DDT '
    .ds 11
;
KEYDT1: 
    .dw 0xF500      ;8009  No Shift
    .ascii  '-0;'       ;'
    .dw 0x6F6C      ;LO
    .ascii  '9'     ;9
    .dw 0xF400      ;8008
    .db 0x1F
    .ascii  ':/'        ;/
    .dw 0x696B      ;KI
    .ascii  '8'
    .dw 0xF300      ;8007
    .dw 0x701E      ;P
    .ascii  '.'     ;★원래는3F(?)였음
    .dw 0x756A      ;JU
    .ascii  '7'
    .dw 0xF200      ;8006
    .dw 0x7840      ;X
    .ascii  ','
    .dw 0x7968      ;HV
    .ascii  '6'
    .dw 0xF100      ;8005
    .dw 0x5D1D
    .dw 0x676D      ;MG
    .dw 0x3574      ;T5
    .dw 0x0000      ;8004
    .dw 0x7C1C
    .dw 0x666E      ;NF
    .dw 0x3472      ;R4
    .dw 0x0008      ;DEL ;8003
    .dw 0x5B1B
    .dw 0x6462      ;BD
    .dw 0x3365      ;E3
    .dw 0x0016      ;LOCK ;8002
    .dw 0x5D7A      ;Z^
    .dw 0x7376      ;VS
    .dw 0x3277      ;W2
    .dw 0x0B5E      ;HOME ;8001
    .dw 0x0D20
    .dw 0x6163      ;CA
    .dw 0x3171      ;Q1
;
;
;
KEYDT2: 
    .dw 0xFA00  ; Shift
    .ascii  '=0+LO)'

    .dw 0xF900
    .db 0x1F
    .ascii  '*?KI('

    .dw 0xF800
    .db 0x1E
    .ascii  'P>JU'
    .db 0x27
    .dw 0xF700
    .db 0x60
    .ascii  'X<HY&'

    .dw 0xF600
    .dw 0x7D1D
    .ascii  'MGT%'
    .dw 0x0000
    .dw 0x7F1C
    .ascii  'NFR$'
    .dw 0x0012      ;INST
    .dw 0x7B1B
    .ascii  'BDE#'
    .dw 0x0017      ;LOCK
    .ascii  'Z'
    .db 0x7D
    .ascii  'VSW"'
    .dw 0x0C7E      ;CLR
    .dw 0x0D20
    .ascii  'CAQ!'  
