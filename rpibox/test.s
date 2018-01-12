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
	ld  a, #0
	ld  bc, #0xc000
LOOP:
	out (c), a
	inc a
	jr nz, LOOP
	ret