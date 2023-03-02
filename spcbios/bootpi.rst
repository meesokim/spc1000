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
   CB03 DD 21 DF F9   [14]   54     ld  ix, #DSKIX
   CB07 DD F9         [10]   55     ld  sp, ix
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 2.
Hexadecimal [16-Bits]



   CB09 F3            [ 4]   56     di
   CB0A AF            [ 4]   57 	xor a
   CB0B 6F            [ 4]   58 	ld  l, a
   CB0C 01 02 00      [10]   59     ld  bc, #0x002
   CB0F 26 0F         [ 7]   60     ld  h,  #15
   CB11 11 00 CC      [10]   61     ld  de, #BIOS
   CB14 CD 3E CB      [17]   62     call _sd_load
   CB17 EB            [ 4]   63 	ex  de, hl
   CB18 01 01 01      [10]   64 	ld  bc, #0x101
   CB1B 26 09         [ 7]   65 	ld  h, #9
   CB1D CD 3E CB      [17]   66     call _sd_load
   CB20 21 00 CC      [10]   67 	ld hl, #MAIN
   CB23 22 01 00      [16]   68 	ld (#0x1), hl
   CB26 21 10 E1      [10]   69 	ld hl, #0xe110
   CB29 11 14 01      [10]   70 	ld de, #0x114
   CB2C 01 6A 02      [10]   71 	ld bc, #618
   CB2F ED B0         [21]   72 	ldir
   CB31 CD D2 CB      [17]   73 	call BASICPATCH
   CB34 CD EA 1B      [17]   74 	call GSAVES
   CB37 AF            [ 4]   75 	xor a
                             76 ;	call SCRNS0
   CB38 21 00 FF      [10]   77 	ld hl, #0xFF00
   CB3B C3 00 CC      [10]   78 	jp MAIN
                             79     
   CB3E                      80 _sd_load:
   CB3E E5            [11]   81     push hl ; size
   CB3F D5            [11]   82     push de ; address
   CB40 C5            [11]   83     push bc ; pos
   CB41 16 02         [ 7]   84     ld  d, #SDREAD
   CB43 CD 6C CB      [17]   85     call sendcmd
   CB46 54            [ 4]   86     ld  d, h
   CB47 CD 74 CB      [17]   87     call senddata
   CB4A 16 00         [ 7]   88     ld  d, #0
   CB4C CD 74 CB      [17]   89     call senddata
   CB4F E1            [10]   90     pop hl
   CB50 54            [ 4]   91     ld  d, h
   CB51 CD 74 CB      [17]   92     call senddata
   CB54 55            [ 4]   93     ld  d, l
   CB55 CD 74 CB      [17]   94     call senddata
   CB58 16 03         [ 7]   95     ld  d, #SDSEND
   CB5A CD 6C CB      [17]   96     call sendcmd
   CB5D E1            [10]   97     pop hl
   CB5E C1            [10]   98     pop bc
   CB5F 0E 00         [ 7]   99     ld  c,#0
   CB61                     100 RDLOOPx:
   CB61 CD A3 CB      [17]  101     call recvdata
   CB64 72            [ 7]  102     ld (hl), d
   CB65 23            [ 6]  103     inc hl
   CB66 0B            [ 6]  104     dec bc
   CB67 78            [ 4]  105     ld  a, b
   CB68 B1            [ 4]  106     or  c
   CB69 20 F6         [12]  107     jr nz, RDLOOPx
   CB6B C9            [10]  108     ret 
                            109 
   CB6C                     110 sendcmd:
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 3.
Hexadecimal [16-Bits]



   CB6C 06 C0         [ 7]  111     LD  B,#0xC0             
   CB6E 0E 02         [ 7]  112     LD  C,#0x02             
   CB70 3E 80         [ 7]  113     LD  A,#0x80             
   CB72 ED 79         [12]  114     OUT (C),A           
   CB74                     115 senddata:   
   CB74 06 C0         [ 7]  116     LD  B,#0xC0             
   CB76 0E 02         [ 7]  117     LD  C,#0x02             
   CB78                     118 CHKRFD1:    
   CB78 ED 78         [12]  119     IN  A,(C)           
   CB7A E6 02         [ 7]  120     AND #0x02           
   CB7C 28 FA         [12]  121     JR  Z,CHKRFD1       
   CB7E 0E 02         [ 7]  122     LD  C,#0x02             
   CB80 AF            [ 4]  123     XOR A               
   CB81 ED 79         [12]  124     OUT (C),A           
   CB83 0E 00         [ 7]  125     LD  C,#0x00             
   CB85 ED 51         [12]  126     OUT (C),D           
   CB87 0E 02         [ 7]  127     LD  C,#0x02             
   CB89 3E 10         [ 7]  128     LD  A,#0x10             
   CB8B ED 79         [12]  129     OUT (C),A           
   CB8D 0E 02         [ 7]  130     LD  C,#0x02         
   CB8F                     131 CHKDAC2:    
   CB8F ED 78         [12]  132     IN  A,(C)   
   CB91 E6 04         [ 7]  133     AND #0x04           
   CB93 28 FA         [12]  134     JR  Z,CHKDAC2       
   CB95 0E 02         [ 7]  135     LD  C,#0x02         
   CB97 AF            [ 4]  136     XOR A             
   CB98 ED 79         [12]  137     OUT (C),A           
   CB9A 0E 02         [ 7]  138     LD  C,#0x02         
   CB9C                     139 CHKDAC3:    
   CB9C ED 78         [12]  140     IN  A,(C)          
   CB9E E6 04         [ 7]  141     AND #0x04           
   CBA0 20 FA         [12]  142     JR  NZ,CHKDAC3      
   CBA2 C9            [10]  143     RET               
                            144     
   CBA3                     145 recvdata:
   CBA3 C5            [11]  146     PUSH    BC           
   CBA4 0E 02         [ 7]  147     LD  C,#0x02             
   CBA6 06 C0         [ 7]  148     LD  B,#0xC0             
   CBA8 3E 20         [ 7]  149     LD  A,#0x20             
   CBAA ED 79         [12]  150     OUT (C),A           
   CBAC 0E 02         [ 7]  151     LD  C,#0x02             
   CBAE                     152 CHKDAV0:    
   CBAE ED 78         [12]  153     IN  A,(C)           
   CBB0 E6 01         [ 7]  154     AND #0x01           
   CBB2 28 FA         [12]  155     JR  Z,CHKDAV0       
   CBB4 0E 02         [ 7]  156     LD  C,#0x02         
   CBB6 AF            [ 4]  157     XOR A               
   CBB7 ED 79         [12]  158     OUT (C),A           
   CBB9 0E 01         [ 7]  159     LD  C,#0x01             
   CBBB ED 50         [12]  160     IN  D,(C)           
   CBBD 0E 02         [ 7]  161     LD  C,#0x02             
   CBBF 3E 40         [ 7]  162     LD  A,#0x40             
   CBC1 ED 79         [12]  163     OUT (C),A         
   CBC3 0E 02         [ 7]  164     LD  C,#0x02             
   CBC5                     165 CHKDAV1:    
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 4.
Hexadecimal [16-Bits]



   CBC5 ED 78         [12]  166     IN  A,(C)           
   CBC7 E6 01         [ 7]  167     AND #0x01           
   CBC9 20 FA         [12]  168     JR  NZ,CHKDAV1      
   CBCB 0E 02         [ 7]  169     LD  C,#0x02         
   CBCD AF            [ 4]  170     XOR A               
   CBCE ED 79         [12]  171     OUT (C),A           
   CBD0 C1            [10]  172     POP BC              
   CBD1 C9            [10]  173     RET           
                            174 
   CBD2                     175 BASICPATCH:	
   CBD2 06 9D         [ 7]  176 	LD  B,#0x09D                          ;ff0d  06 9d          531   2687 ; 1. replace 7c4e --> 7c9d from address 04300h to 01500h  
   CBD4 21 00 43      [10]  177     LD  HL,#0x04300                       ;ff0f  21 00 43       532   2688 ;
   CBD7 7E            [ 7]  178 L0FF0Ah:    LD  A,(HL)                  ;ff12  7e             533   2689 ;
   CBD8 FE 7C         [ 7]  179     CP  #0x7C                            ;ff13  fe 7c          534   2690 ;
   CBDA 20 07         [12]  180     JR  NZ,L0FF16h                      ;ff15  20 07          535   2691 ; 
   CBDC 2B            [ 6]  181     DEC HL                              ;ff17  2b             536   2692 ;
   CBDD 7E            [ 7]  182     LD  A,(HL)                          ;ff18  7e             537   2693 ;
   CBDE FE 4E         [ 7]  183     CP  #0x4E                            ;ff19  fe 4e          538   2694 ;
   CBE0 20 01         [12]  184     JR  NZ,L0FF16h                      ;ff1b  20 01          539   2695 ; 
   CBE2 70            [ 7]  185     LD  (HL),B                          ;ff1d  70             540   2696 ;
   CBE3 2B            [ 6]  186 L0FF16h:    DEC HL                      ;ff1e  2b             541   2697 ;
   CBE4 7C            [ 4]  187     LD  A,H                             ;ff1f  7c             542   2698 ;
   CBE5 FE 15         [ 7]  188     CP  #0x15                            ;ff20  fe 15          543   2699 ;
   CBE7 30 EE         [12]  189     JR  NC,L0FF0Ah                      ;ff22  30 ee          544   2700 ;
   CBE9 21 3B 7A      [10]  190     LD  HL,#0x7A3B                       ;ff24  21 3b 7a       545   2701 ; 2. put data 09dh at address 7a3bh
   CBEC 70            [ 7]  191     LD  (HL),B                          ;ff27  70             546   2702 ;
   CBED C9            [10]  192 	RET
                            193 
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 5.
Hexadecimal [16-Bits]

Symbol Table

    .__.$$$.=  2710 L   |     .__.ABS.=  0000 G   |     .__.CPU.=  0000 L
    .__.H$L.=  0000 L   |   5 BASICPAT   CBD2 R   |     BDOS    =  D406 
    BIAS    =  9800     |     BIOS    =  CC00     |     BIOSL   =  0300 
    BOOTBAS =  FB23     |     CCP     =  CC00     |     CDISK   =  0004 
    CGINT   =  554A     |   5 CHKDAC2    CB8F R   |   5 CHKDAC3    CB9C R
  5 CHKDAV0    CBAE R   |   5 CHKDAV1    CBC5 R   |   5 CHKRFD1    CB78 R
    CLR02   =  0AD5     |     CLS     =  1B42     |     CPM_BOOT=  CB00 
    DEPRINT =  07F3     |     DEPRT   =  07F3     |     DSKIX   =  F9DF 
    GRAPH   =  1B95     |     GSAVES  =  1BEA     |     INITSB  =  0050 
    IOBYTE  =  0003     |   5 L0FF0Ah    CBD7 R   |   5 L0FF16h    CBE3 R
    MAIN    =  CC00     |     MEMSET  =  33E9     |     MSIZE   =  003A 
    NSECTS  =  0000     |   5 RDLOOPx    CB61 R   |     ROMPATCH=  FB29 
    SCRNS0  =  05F3     |     SDCOPY  =  0004     |     SDFORMAT=  0005 
    SDREAD  =  0002     |     SDSEND  =  0003     |     SDWRITE =  0001 
    SECTS   =  0006     |     SIZE    =  0300     |     TICONT  =  0036 
  5 _sd_load   CB3E R   |   5 recvdata   CBA3 R   |   5 sendcmd    CB6C R
  5 senddata   CB74 R   |   5 start      CB00 GR

ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 6.
Hexadecimal [16-Bits]

Area Table

   0 _CODE      size    0   flags    0
   1 HOME       size    0   flags    0
   2 XSEG       size    0   flags    0
   3 PSEG       size    0   flags    0
   4 _HEADER    size    0   flags    8
   5 _HEADER0   size   EE   flags    8

