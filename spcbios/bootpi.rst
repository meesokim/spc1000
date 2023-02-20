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
   CB00                      51 start::
                             52 ;   .ascii "SYS"
   CB00 DD 21 DF F9   [14]   53     ld  ix, #DSKIX
   CB04 DD F9         [10]   54     ld  sp, ix
   CB06 F3            [ 4]   55     di
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 2.
Hexadecimal [16-Bits]



   CB07 AF            [ 4]   56 	xor a
   CB08 6F            [ 4]   57 	ld  l, a
   CB09 01 02 00      [10]   58     ld  bc, #0x002
   CB0C 26 0F         [ 7]   59     ld  h,  #15
   CB0E 11 00 CC      [10]   60     ld  de, #BIOS
   CB11 CD 3B CB      [17]   61     call _sd_load
   CB14 EB            [ 4]   62 	ex  de, hl
   CB15 01 01 01      [10]   63 	ld  bc, #0x101
   CB18 26 09         [ 7]   64 	ld  h, #9
   CB1A CD 3B CB      [17]   65     call _sd_load
   CB1D 21 00 CC      [10]   66 	ld hl, #MAIN
   CB20 22 01 00      [16]   67 	ld (#0x1), hl
   CB23 21 10 E0      [10]   68 	ld hl, #0xcb00 + #0x1510
   CB26 11 14 01      [10]   69 	ld de, #0x114
   CB29 01 72 02      [10]   70 	ld bc, #634-#8
   CB2C ED B0         [21]   71 	ldir
   CB2E CD CF CB      [17]   72 	call BASICPATCH
   CB31 CD EA 1B      [17]   73 	call GSAVES
   CB34 AF            [ 4]   74 	xor a
                             75 ;	call SCRNS0
   CB35 21 00 FF      [10]   76 	ld hl, #0xFF00
   CB38 C3 00 CC      [10]   77 	jp MAIN
                             78     
   CB3B                      79 _sd_load:
   CB3B E5            [11]   80     push hl ; size
   CB3C D5            [11]   81     push de ; address
   CB3D C5            [11]   82     push bc ; pos
   CB3E 16 02         [ 7]   83     ld  d, #SDREAD
   CB40 CD 69 CB      [17]   84     call sendcmd
   CB43 54            [ 4]   85     ld  d, h
   CB44 CD 71 CB      [17]   86     call senddata
   CB47 16 00         [ 7]   87     ld  d, #0
   CB49 CD 71 CB      [17]   88     call senddata
   CB4C E1            [10]   89     pop hl
   CB4D 54            [ 4]   90     ld  d, h
   CB4E CD 71 CB      [17]   91     call senddata
   CB51 55            [ 4]   92     ld  d, l
   CB52 CD 71 CB      [17]   93     call senddata
   CB55 16 03         [ 7]   94     ld  d, #SDSEND
   CB57 CD 69 CB      [17]   95     call sendcmd
   CB5A E1            [10]   96     pop hl
   CB5B C1            [10]   97     pop bc
   CB5C 0E 00         [ 7]   98     ld  c,#0
   CB5E                      99 RDLOOPx:
   CB5E CD A0 CB      [17]  100     call recvdata
   CB61 72            [ 7]  101     ld (hl), d
   CB62 23            [ 6]  102     inc hl
   CB63 0B            [ 6]  103     dec bc
   CB64 78            [ 4]  104     ld  a, b
   CB65 B1            [ 4]  105     or  c
   CB66 20 F6         [12]  106     jr nz, RDLOOPx
   CB68 C9            [10]  107     ret 
                            108 
   CB69                     109 sendcmd:
   CB69 06 C0         [ 7]  110     LD  B,#0xC0             
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 3.
Hexadecimal [16-Bits]



   CB6B 0E 02         [ 7]  111     LD  C,#0x02             
   CB6D 3E 80         [ 7]  112     LD  A,#0x80             
   CB6F ED 79         [12]  113     OUT (C),A           
   CB71                     114 senddata:   
   CB71 06 C0         [ 7]  115     LD  B,#0xC0             
   CB73 0E 02         [ 7]  116     LD  C,#0x02             
   CB75                     117 CHKRFD1:    
   CB75 ED 78         [12]  118     IN  A,(C)           
   CB77 E6 02         [ 7]  119     AND #0x02           
   CB79 28 FA         [12]  120     JR  Z,CHKRFD1       
   CB7B 0E 02         [ 7]  121     LD  C,#0x02             
   CB7D AF            [ 4]  122     XOR A               
   CB7E ED 79         [12]  123     OUT (C),A           
   CB80 0E 00         [ 7]  124     LD  C,#0x00             
   CB82 ED 51         [12]  125     OUT (C),D           
   CB84 0E 02         [ 7]  126     LD  C,#0x02             
   CB86 3E 10         [ 7]  127     LD  A,#0x10             
   CB88 ED 79         [12]  128     OUT (C),A           
   CB8A 0E 02         [ 7]  129     LD  C,#0x02         
   CB8C                     130 CHKDAC2:    
   CB8C ED 78         [12]  131     IN  A,(C)   
   CB8E E6 04         [ 7]  132     AND #0x04           
   CB90 28 FA         [12]  133     JR  Z,CHKDAC2       
   CB92 0E 02         [ 7]  134     LD  C,#0x02         
   CB94 AF            [ 4]  135     XOR A             
   CB95 ED 79         [12]  136     OUT (C),A           
   CB97 0E 02         [ 7]  137     LD  C,#0x02         
   CB99                     138 CHKDAC3:    
   CB99 ED 78         [12]  139     IN  A,(C)          
   CB9B E6 04         [ 7]  140     AND #0x04           
   CB9D 20 FA         [12]  141     JR  NZ,CHKDAC3      
   CB9F C9            [10]  142     RET               
                            143     
   CBA0                     144 recvdata:
   CBA0 C5            [11]  145     PUSH    BC           
   CBA1 0E 02         [ 7]  146     LD  C,#0x02             
   CBA3 06 C0         [ 7]  147     LD  B,#0xC0             
   CBA5 3E 20         [ 7]  148     LD  A,#0x20             
   CBA7 ED 79         [12]  149     OUT (C),A           
   CBA9 0E 02         [ 7]  150     LD  C,#0x02             
   CBAB                     151 CHKDAV0:    
   CBAB ED 78         [12]  152     IN  A,(C)           
   CBAD E6 01         [ 7]  153     AND #0x01           
   CBAF 28 FA         [12]  154     JR  Z,CHKDAV0       
   CBB1 0E 02         [ 7]  155     LD  C,#0x02         
   CBB3 AF            [ 4]  156     XOR A               
   CBB4 ED 79         [12]  157     OUT (C),A           
   CBB6 0E 01         [ 7]  158     LD  C,#0x01             
   CBB8 ED 50         [12]  159     IN  D,(C)           
   CBBA 0E 02         [ 7]  160     LD  C,#0x02             
   CBBC 3E 40         [ 7]  161     LD  A,#0x40             
   CBBE ED 79         [12]  162     OUT (C),A         
   CBC0 0E 02         [ 7]  163     LD  C,#0x02             
   CBC2                     164 CHKDAV1:    
   CBC2 ED 78         [12]  165     IN  A,(C)           
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 4.
Hexadecimal [16-Bits]



   CBC4 E6 01         [ 7]  166     AND #0x01           
   CBC6 20 FA         [12]  167     JR  NZ,CHKDAV1      
   CBC8 0E 02         [ 7]  168     LD  C,#0x02         
   CBCA AF            [ 4]  169     XOR A               
   CBCB ED 79         [12]  170     OUT (C),A           
   CBCD C1            [10]  171     POP BC              
   CBCE C9            [10]  172     RET           
                            173 
   CBCF                     174 BASICPATCH:	
   CBCF 06 9D         [ 7]  175 	LD  B,#0x09D                          ;ff0d  06 9d          531   2687 ; 1. replace 7c4e --> 7c9d from address 04300h to 01500h  
   CBD1 21 00 43      [10]  176     LD  HL,#0x04300                       ;ff0f  21 00 43       532   2688 ;
   CBD4 7E            [ 7]  177 L0FF0Ah:    LD  A,(HL)                  ;ff12  7e             533   2689 ;
   CBD5 FE 7C         [ 7]  178     CP  #0x7C                            ;ff13  fe 7c          534   2690 ;
   CBD7 20 07         [12]  179     JR  NZ,L0FF16h                      ;ff15  20 07          535   2691 ; 
   CBD9 2B            [ 6]  180     DEC HL                              ;ff17  2b             536   2692 ;
   CBDA 7E            [ 7]  181     LD  A,(HL)                          ;ff18  7e             537   2693 ;
   CBDB FE 4E         [ 7]  182     CP  #0x4E                            ;ff19  fe 4e          538   2694 ;
   CBDD 20 01         [12]  183     JR  NZ,L0FF16h                      ;ff1b  20 01          539   2695 ; 
   CBDF 70            [ 7]  184     LD  (HL),B                          ;ff1d  70             540   2696 ;
   CBE0 2B            [ 6]  185 L0FF16h:    DEC HL                      ;ff1e  2b             541   2697 ;
   CBE1 7C            [ 4]  186     LD  A,H                             ;ff1f  7c             542   2698 ;
   CBE2 FE 15         [ 7]  187     CP  #0x15                            ;ff20  fe 15          543   2699 ;
   CBE4 30 EE         [12]  188     JR  NC,L0FF0Ah                      ;ff22  30 ee          544   2700 ;
   CBE6 21 3B 7A      [10]  189     LD  HL,#0x7A3B                       ;ff24  21 3b 7a       545   2701 ; 2. put data 09dh at address 7a3bh
   CBE9 70            [ 7]  190     LD  (HL),B                          ;ff27  70             546   2702 ;
   CBEA C9            [10]  191 	RET
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 5.
Hexadecimal [16-Bits]

Symbol Table

    .__.$$$.=  2710 L   |     .__.ABS.=  0000 G   |     .__.CPU.=  0000 L
    .__.H$L.=  0000 L   |   5 BASICPAT   CBCF R   |     BDOS    =  D406 
    BIAS    =  9800     |     BIOS    =  CC00     |     BIOSL   =  0300 
    BOOTBAS =  FB23     |     CCP     =  CC00     |     CDISK   =  0004 
    CGINT   =  554A     |   5 CHKDAC2    CB8C R   |   5 CHKDAC3    CB99 R
  5 CHKDAV0    CBAB R   |   5 CHKDAV1    CBC2 R   |   5 CHKRFD1    CB75 R
    CLR02   =  0AD5     |     CLS     =  1B42     |     CPM_BOOT=  CB00 
    DEPRT   =  07F3     |     DSKIX   =  F9DF     |     GRAPH   =  1B95 
    GSAVES  =  1BEA     |     INITSB  =  0050     |     IOBYTE  =  0003 
  5 L0FF0Ah    CBD4 R   |   5 L0FF16h    CBE0 R   |     MAIN    =  CC00 
    MEMSET  =  33E9     |     MSIZE   =  003A     |     NSECTS  =  0000 
  5 RDLOOPx    CB5E R   |     ROMPATCH=  FB29     |     SCRNS0  =  05F3 
    SDCOPY  =  0004     |     SDFORMAT=  0005     |     SDREAD  =  0002 
    SDSEND  =  0003     |     SDWRITE =  0001     |     SECTS   =  0006 
    SIZE    =  0300     |     TICONT  =  0036     |   5 _sd_load   CB3B R
  5 recvdata   CBA0 R   |   5 sendcmd    CB69 R   |   5 senddata   CB71 R
  5 start      CB00 GR

ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 6.
Hexadecimal [16-Bits]

Area Table

   0 _CODE      size    0   flags    0
   1 HOME       size    0   flags    0
   2 XSEG       size    0   flags    0
   3 PSEG       size    0   flags    0
   4 _HEADER    size    0   flags    8
   5 _HEADER0   size   EB   flags    8

