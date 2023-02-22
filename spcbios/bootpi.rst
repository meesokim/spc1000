ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 1.
Hexadecimal [16-Bits]



                              1 ;==============================================================================
                              2 ;   Raspberry Pi booting code for SPC-1000
                              3 ;
                              4 ;           bootpi.s
                              5 ;                                   By meeso.kim
                              6 ;==============================================================================
                              7 
                              8     .module boot
                              9     .area   _CODE
                             10 	.area   HOME
                             11 	.area   XSEG
                             12 	.area   PSEG
                             13     .area  _HEADER  (ABS)
   CB00                      14     .org    CPM_BOOT
                             15     
                     003A    16 MSIZE   =   58
                     9800    17 BIAS    =   (MSIZE-20)*1024
                     CC00    18 CCP     =   0x3400+BIAS
                     CC00    19 BIOS    =   CPM_BOOT+0x100
                     0300    20 BIOSL   =   0x0300      ;length of the bios
                     D406    21 BDOS    =   CCP+0x806   ;base of bdos
                             22 
                             23 ;BOOT   =   BIOS
                     0300    24 SIZE    =   BIOS+BIOSL-CCP  ;size of cp/m system
                     0006    25 SECTS   =   SIZE/128    ;# of sectors to load
                     0000    26 NSECTS  =   (BIOS-CCP)/128
                     F9DF    27 DSKIX   =   0x0fa00-33
                     1B42    28 CLS     =   0x1b42
                     0AD5    29 CLR02   =   0x0ad5
                     1BEA    30 GSAVES  =   0x1bea
                     0004    31 CDISK   =   0x0004      ;current disk number 0=A,...,15=P
                     0003    32 IOBYTE  =   0x0003      ;intel i/o byte
                     0036    33 TICONT  =   0x0036
                     07F3    34 DEPRT   =   0x07F3
                     554A    35 CGINT	=	0x554A
                             36 
                     0001    37 SDWRITE     = 1
                     0002    38 SDREAD      = 2
                     0003    39 SDSEND      = 3
                     0004    40 SDCOPY      = 4
                     0005    41 SDFORMAT    = 5
                             42             
                     CB00    43 CPM_BOOT   =   0xcb00 
                     CC00    44 MAIN  	   =   0xcc00  
                     FB29    45 ROMPATCH   =   0xfb29
                     FB23    46 BOOTBAS	   =   0xfb23
                     33E9    47 MEMSET	   =   0x33e9
                     0050    48 INITSB	   =   0x0050
                     1B95    49 GRAPH	   =   0x1b95
                     05F3    50 SCRNS0	   =   0x05f3
                     07F3    51 DEPRINT    =   0x07f3
   CB00                      52 start::
   CB00 CD 42 1B      [17]   53     call CLS
   CB03 11 F4 CB      [10]   54    	LD	DE, #TITLEMSG      	;N
   CB06 CD F3 07      [17]   55   	CALL	DEPRINT 
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 2.
Hexadecimal [16-Bits]



   CB09 DD 21 DF F9   [14]   56     ld  ix, #DSKIX
   CB0D DD F9         [10]   57     ld  sp, ix
   CB0F F3            [ 4]   58     di
   CB10 AF            [ 4]   59 	xor a
   CB11 6F            [ 4]   60 	ld  l, a
   CB12 01 02 00      [10]   61     ld  bc, #0x002
   CB15 26 0F         [ 7]   62     ld  h,  #15
   CB17 11 00 CC      [10]   63     ld  de, #BIOS
   CB1A CD 44 CB      [17]   64     call _sd_load
   CB1D EB            [ 4]   65 	ex  de, hl
   CB1E 01 01 01      [10]   66 	ld  bc, #0x101
   CB21 26 09         [ 7]   67 	ld  h, #9
   CB23 CD 44 CB      [17]   68     call _sd_load
   CB26 21 00 CC      [10]   69 	ld hl, #MAIN
   CB29 22 01 00      [16]   70 	ld (#0x1), hl
   CB2C 21 10 E0      [10]   71 	ld hl, #0xcb00 + #0x1510
   CB2F 11 14 01      [10]   72 	ld de, #0x114
   CB32 01 72 02      [10]   73 	ld bc, #634-#8
   CB35 ED B0         [21]   74 	ldir
   CB37 CD D8 CB      [17]   75 	call BASICPATCH
   CB3A CD EA 1B      [17]   76 	call GSAVES
   CB3D AF            [ 4]   77 	xor a
                             78 ;	call SCRNS0
   CB3E 21 00 FF      [10]   79 	ld hl, #0xFF00
   CB41 C3 00 CC      [10]   80 	jp MAIN
                             81     
   CB44                      82 _sd_load:
   CB44 E5            [11]   83     push hl ; size
   CB45 D5            [11]   84     push de ; address
   CB46 C5            [11]   85     push bc ; pos
   CB47 16 02         [ 7]   86     ld  d, #SDREAD
   CB49 CD 72 CB      [17]   87     call sendcmd
   CB4C 54            [ 4]   88     ld  d, h
   CB4D CD 7A CB      [17]   89     call senddata
   CB50 16 00         [ 7]   90     ld  d, #0
   CB52 CD 7A CB      [17]   91     call senddata
   CB55 E1            [10]   92     pop hl
   CB56 54            [ 4]   93     ld  d, h
   CB57 CD 7A CB      [17]   94     call senddata
   CB5A 55            [ 4]   95     ld  d, l
   CB5B CD 7A CB      [17]   96     call senddata
   CB5E 16 03         [ 7]   97     ld  d, #SDSEND
   CB60 CD 72 CB      [17]   98     call sendcmd
   CB63 E1            [10]   99     pop hl
   CB64 C1            [10]  100     pop bc
   CB65 0E 00         [ 7]  101     ld  c,#0
   CB67                     102 RDLOOPx:
   CB67 CD A9 CB      [17]  103     call recvdata
   CB6A 72            [ 7]  104     ld (hl), d
   CB6B 23            [ 6]  105     inc hl
   CB6C 0B            [ 6]  106     dec bc
   CB6D 78            [ 4]  107     ld  a, b
   CB6E B1            [ 4]  108     or  c
   CB6F 20 F6         [12]  109     jr nz, RDLOOPx
   CB71 C9            [10]  110     ret 
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 3.
Hexadecimal [16-Bits]



                            111 
   CB72                     112 sendcmd:
   CB72 06 C0         [ 7]  113     LD  B,#0xC0             
   CB74 0E 02         [ 7]  114     LD  C,#0x02             
   CB76 3E 80         [ 7]  115     LD  A,#0x80             
   CB78 ED 79         [12]  116     OUT (C),A           
   CB7A                     117 senddata:   
   CB7A 06 C0         [ 7]  118     LD  B,#0xC0             
   CB7C 0E 02         [ 7]  119     LD  C,#0x02             
   CB7E                     120 CHKRFD1:    
   CB7E ED 78         [12]  121     IN  A,(C)           
   CB80 E6 02         [ 7]  122     AND #0x02           
   CB82 28 FA         [12]  123     JR  Z,CHKRFD1       
   CB84 0E 02         [ 7]  124     LD  C,#0x02             
   CB86 AF            [ 4]  125     XOR A               
   CB87 ED 79         [12]  126     OUT (C),A           
   CB89 0E 00         [ 7]  127     LD  C,#0x00             
   CB8B ED 51         [12]  128     OUT (C),D           
   CB8D 0E 02         [ 7]  129     LD  C,#0x02             
   CB8F 3E 10         [ 7]  130     LD  A,#0x10             
   CB91 ED 79         [12]  131     OUT (C),A           
   CB93 0E 02         [ 7]  132     LD  C,#0x02         
   CB95                     133 CHKDAC2:    
   CB95 ED 78         [12]  134     IN  A,(C)   
   CB97 E6 04         [ 7]  135     AND #0x04           
   CB99 28 FA         [12]  136     JR  Z,CHKDAC2       
   CB9B 0E 02         [ 7]  137     LD  C,#0x02         
   CB9D AF            [ 4]  138     XOR A             
   CB9E ED 79         [12]  139     OUT (C),A           
   CBA0 0E 02         [ 7]  140     LD  C,#0x02         
   CBA2                     141 CHKDAC3:    
   CBA2 ED 78         [12]  142     IN  A,(C)          
   CBA4 E6 04         [ 7]  143     AND #0x04           
   CBA6 20 FA         [12]  144     JR  NZ,CHKDAC3      
   CBA8 C9            [10]  145     RET               
                            146     
   CBA9                     147 recvdata:
   CBA9 C5            [11]  148     PUSH    BC           
   CBAA 0E 02         [ 7]  149     LD  C,#0x02             
   CBAC 06 C0         [ 7]  150     LD  B,#0xC0             
   CBAE 3E 20         [ 7]  151     LD  A,#0x20             
   CBB0 ED 79         [12]  152     OUT (C),A           
   CBB2 0E 02         [ 7]  153     LD  C,#0x02             
   CBB4                     154 CHKDAV0:    
   CBB4 ED 78         [12]  155     IN  A,(C)           
   CBB6 E6 01         [ 7]  156     AND #0x01           
   CBB8 28 FA         [12]  157     JR  Z,CHKDAV0       
   CBBA 0E 02         [ 7]  158     LD  C,#0x02         
   CBBC AF            [ 4]  159     XOR A               
   CBBD ED 79         [12]  160     OUT (C),A           
   CBBF 0E 01         [ 7]  161     LD  C,#0x01             
   CBC1 ED 50         [12]  162     IN  D,(C)           
   CBC3 0E 02         [ 7]  163     LD  C,#0x02             
   CBC5 3E 40         [ 7]  164     LD  A,#0x40             
   CBC7 ED 79         [12]  165     OUT (C),A         
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 4.
Hexadecimal [16-Bits]



   CBC9 0E 02         [ 7]  166     LD  C,#0x02             
   CBCB                     167 CHKDAV1:    
   CBCB ED 78         [12]  168     IN  A,(C)           
   CBCD E6 01         [ 7]  169     AND #0x01           
   CBCF 20 FA         [12]  170     JR  NZ,CHKDAV1      
   CBD1 0E 02         [ 7]  171     LD  C,#0x02         
   CBD3 AF            [ 4]  172     XOR A               
   CBD4 ED 79         [12]  173     OUT (C),A           
   CBD6 C1            [10]  174     POP BC              
   CBD7 C9            [10]  175     RET           
                            176 
   CBD8                     177 BASICPATCH:	
   CBD8 06 9D         [ 7]  178 	LD  B,#0x09D                          ;ff0d  06 9d          531   2687 ; 1. replace 7c4e --> 7c9d from address 04300h to 01500h  
   CBDA 21 00 43      [10]  179     LD  HL,#0x04300                       ;ff0f  21 00 43       532   2688 ;
   CBDD 7E            [ 7]  180 L0FF0Ah:    LD  A,(HL)                  ;ff12  7e             533   2689 ;
   CBDE FE 7C         [ 7]  181     CP  #0x7C                            ;ff13  fe 7c          534   2690 ;
   CBE0 20 07         [12]  182     JR  NZ,L0FF16h                      ;ff15  20 07          535   2691 ; 
   CBE2 2B            [ 6]  183     DEC HL                              ;ff17  2b             536   2692 ;
   CBE3 7E            [ 7]  184     LD  A,(HL)                          ;ff18  7e             537   2693 ;
   CBE4 FE 4E         [ 7]  185     CP  #0x4E                            ;ff19  fe 4e          538   2694 ;
   CBE6 20 01         [12]  186     JR  NZ,L0FF16h                      ;ff1b  20 01          539   2695 ; 
   CBE8 70            [ 7]  187     LD  (HL),B                          ;ff1d  70             540   2696 ;
   CBE9 2B            [ 6]  188 L0FF16h:    DEC HL                      ;ff1e  2b             541   2697 ;
   CBEA 7C            [ 4]  189     LD  A,H                             ;ff1f  7c             542   2698 ;
   CBEB FE 15         [ 7]  190     CP  #0x15                            ;ff20  fe 15          543   2699 ;
   CBED 30 EE         [12]  191     JR  NC,L0FF0Ah                      ;ff22  30 ee          544   2700 ;
   CBEF 21 3B 7A      [10]  192     LD  HL,#0x7A3B                       ;ff24  21 3b 7a       545   2701 ; 2. put data 09dh at address 7a3bh
   CBF2 70            [ 7]  193     LD  (HL),B                          ;ff27  70             546   2702 ;
   CBF3 C9            [10]  194 	RET
                            195 
   CBF4                     196 TITLEMSG:
   CBF4 4D 45 53 53 41 47   197     .ascii /MESSAGE/
        45
   CBFB 00                  198     .byte 0
                            199 
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 5.
Hexadecimal [16-Bits]

Symbol Table

    .__.$$$.=  2710 L   |     .__.ABS.=  0000 G   |     .__.CPU.=  0000 L
    .__.H$L.=  0000 L   |   5 BASICPAT   CBD8 R   |     BDOS    =  D406 
    BIAS    =  9800     |     BIOS    =  CC00     |     BIOSL   =  0300 
    BOOTBAS =  FB23     |     CCP     =  CC00     |     CDISK   =  0004 
    CGINT   =  554A     |   5 CHKDAC2    CB95 R   |   5 CHKDAC3    CBA2 R
  5 CHKDAV0    CBB4 R   |   5 CHKDAV1    CBCB R   |   5 CHKRFD1    CB7E R
    CLR02   =  0AD5     |     CLS     =  1B42     |     CPM_BOOT=  CB00 
    DEPRINT =  07F3     |     DEPRT   =  07F3     |     DSKIX   =  F9DF 
    GRAPH   =  1B95     |     GSAVES  =  1BEA     |     INITSB  =  0050 
    IOBYTE  =  0003     |   5 L0FF0Ah    CBDD R   |   5 L0FF16h    CBE9 R
    MAIN    =  CC00     |     MEMSET  =  33E9     |     MSIZE   =  003A 
    NSECTS  =  0000     |   5 RDLOOPx    CB67 R   |     ROMPATCH=  FB29 
    SCRNS0  =  05F3     |     SDCOPY  =  0004     |     SDFORMAT=  0005 
    SDREAD  =  0002     |     SDSEND  =  0003     |     SDWRITE =  0001 
    SECTS   =  0006     |     SIZE    =  0300     |     TICONT  =  0036 
  5 TITLEMSG   CBF4 R   |   5 _sd_load   CB44 R   |   5 recvdata   CBA9 R
  5 sendcmd    CB72 R   |   5 senddata   CB7A R   |   5 start      CB00 GR

ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 6.
Hexadecimal [16-Bits]

Area Table

   0 _CODE      size    0   flags    0
   1 HOME       size    0   flags    0
   2 XSEG       size    0   flags    0
   3 PSEG       size    0   flags    0
   4 _HEADER    size    0   flags    8
   5 _HEADER0   size   FC   flags    8

