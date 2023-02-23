;==============================================================================
;   Raspberry Pi booting code for SPC-1000
;
;           bootpi.s
;                                   By meeso.kim
;==============================================================================

    .module boot
    .area   _CODE
	.area   HOME
	.area   XSEG
	.area   PSEG
    .area  _HEADER  (ABS)
    .org    CPM_BOOT
    
MSIZE   =   58
BIAS    =   (MSIZE-20)*1024
CCP     =   0x3400+BIAS
BIOS    =   CPM_BOOT+0x100
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
DEPRT   =   0x07F3
CGINT	=	0x554A

SDWRITE     = 1
SDREAD      = 2
SDSEND      = 3
SDCOPY      = 4
SDFORMAT    = 5
            
CPM_BOOT   =   0xcb00 
MAIN  	   =   0xcc00  
ROMPATCH   =   0xfb29
BOOTBAS	   =   0xfb23
MEMSET	   =   0x33e9
INITSB	   =   0x0050
GRAPH	   =   0x1b95
SCRNS0	   =   0x05f3
DEPRINT    =   0x07f3
start::
    call CLS
    ld  ix, #DSKIX
    ld  sp, ix
    di
	xor a
	ld  l, a
    ld  bc, #0x002
    ld  h,  #15
    ld  de, #BIOS
    call _sd_load
	ex  de, hl
	ld  bc, #0x101
	ld  h, #9
    call _sd_load
	ld hl, #MAIN
	ld (#0x1), hl
	ld hl, #0xcb00 + #0x1510
	ld de, #0x114
	ld bc, #634-#8
	ldir
	call BASICPATCH
	call GSAVES
	xor a
;	call SCRNS0
	ld hl, #0xFF00
	jp MAIN
    
_sd_load:
    push hl ; size
    push de ; address
    push bc ; pos
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

BASICPATCH:	
	LD  B,#0x09D                          ;ff0d  06 9d          531   2687 ; 1. replace 7c4e --> 7c9d from address 04300h to 01500h  
    LD  HL,#0x04300                       ;ff0f  21 00 43       532   2688 ;
L0FF0Ah:    LD  A,(HL)                  ;ff12  7e             533   2689 ;
    CP  #0x7C                            ;ff13  fe 7c          534   2690 ;
    JR  NZ,L0FF16h                      ;ff15  20 07          535   2691 ; 
    DEC HL                              ;ff17  2b             536   2692 ;
    LD  A,(HL)                          ;ff18  7e             537   2693 ;
    CP  #0x4E                            ;ff19  fe 4e          538   2694 ;
    JR  NZ,L0FF16h                      ;ff1b  20 01          539   2695 ; 
    LD  (HL),B                          ;ff1d  70             540   2696 ;
L0FF16h:    DEC HL                      ;ff1e  2b             541   2697 ;
    LD  A,H                             ;ff1f  7c             542   2698 ;
    CP  #0x15                            ;ff20  fe 15          543   2699 ;
    JR  NC,L0FF0Ah                      ;ff22  30 ee          544   2700 ;
    LD  HL,#0x7A3B                       ;ff24  21 3b 7a       545   2701 ; 2. put data 09dh at address 7a3bh
    LD  (HL),B                          ;ff27  70             546   2702 ;
	RET

TITLEMSG:
    .ascii /MESSAGE/
    .byte 0

