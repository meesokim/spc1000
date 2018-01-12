ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 1.
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
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 2.
Hexadecimal [16-Bits]



   CB07 AF            [ 4]   56 	xor a
   CB08 6F            [ 4]   57 	ld  l, a
   CB09 01 02 00      [10]   58     ld  bc, #0x002
   CB0C 26 0F         [ 7]   59     ld  h,  #15
   CB0E 11 00 CC      [10]   60     ld  de, #BIOS
   CB11 CD 38 CB      [17]   61     call _sd_load
   CB14 EB            [ 4]   62 	ex  de, hl
   CB15 01 01 01      [10]   63 	ld  bc, #0x101
   CB18 26 09         [ 7]   64 	ld  h, #9
   CB1A CD 38 CB      [17]   65     call _sd_load
   CB1D 21 00 CC      [10]   66 	ld hl, #MAIN
   CB20 22 01 00      [16]   67 	ld (#0x1), hl
   CB23 21 10 E0      [10]   68 	ld hl, #0xcb00 + #0x1510
   CB26 11 14 01      [10]   69 	ld de, #0x114
   CB29 01 72 02      [10]   70 	ld bc, #634-#8
   CB2C ED B0         [21]   71 	ldir
   CB2E CD EA 1B      [17]   72 	call GSAVES
   CB31 AF            [ 4]   73 	xor a
                             74 ;	call SCRNS0
   CB32 21 00 FF      [10]   75 	ld hl, #0xFF00
   CB35 C3 00 CC      [10]   76 	jp MAIN
                             77     
   CB38                      78 _sd_load:
   CB38 E5            [11]   79     push hl ; size
   CB39 D5            [11]   80     push de ; address
   CB3A C5            [11]   81     push bc ; pos
   CB3B 16 02         [ 7]   82     ld  d, #SDREAD
   CB3D CD 66 CB      [17]   83     call sendcmd
   CB40 54            [ 4]   84     ld  d, h
   CB41 CD 6E CB      [17]   85     call senddata
   CB44 16 00         [ 7]   86     ld  d, #0
   CB46 CD 6E CB      [17]   87     call senddata
   CB49 E1            [10]   88     pop hl
   CB4A 54            [ 4]   89     ld  d, h
   CB4B CD 6E CB      [17]   90     call senddata
   CB4E 55            [ 4]   91     ld  d, l
   CB4F CD 6E CB      [17]   92     call senddata
   CB52 16 03         [ 7]   93     ld  d, #SDSEND
   CB54 CD 66 CB      [17]   94     call sendcmd
   CB57 E1            [10]   95     pop hl
   CB58 C1            [10]   96     pop bc
   CB59 0E 00         [ 7]   97     ld  c,#0
   CB5B                      98 RDLOOPx:
   CB5B CD 9D CB      [17]   99     call recvdata
   CB5E 72            [ 7]  100     ld (hl), d
   CB5F 23            [ 6]  101     inc hl
   CB60 0B            [ 6]  102     dec bc
   CB61 78            [ 4]  103     ld  a, b
   CB62 B1            [ 4]  104     or  c
   CB63 20 F6         [12]  105     jr nz, RDLOOPx
   CB65 C9            [10]  106     ret 
                            107 
   CB66                     108 sendcmd:
   CB66 06 C0         [ 7]  109     LD  B,#0xC0             
   CB68 0E 02         [ 7]  110     LD  C,#0x02             
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 3.
Hexadecimal [16-Bits]



   CB6A 3E 80         [ 7]  111     LD  A,#0x80             
   CB6C ED 79         [12]  112     OUT (C),A           
   CB6E                     113 senddata:   
   CB6E 06 C0         [ 7]  114     LD  B,#0xC0             
   CB70 0E 02         [ 7]  115     LD  C,#0x02             
   CB72                     116 CHKRFD1:    
   CB72 ED 78         [12]  117     IN  A,(C)           
   CB74 E6 02         [ 7]  118     AND #0x02           
   CB76 28 FA         [12]  119     JR  Z,CHKRFD1       
   CB78 0E 02         [ 7]  120     LD  C,#0x02             
   CB7A AF            [ 4]  121     XOR A               
   CB7B ED 79         [12]  122     OUT (C),A           
   CB7D 0E 00         [ 7]  123     LD  C,#0x00             
   CB7F ED 51         [12]  124     OUT (C),D           
   CB81 0E 02         [ 7]  125     LD  C,#0x02             
   CB83 3E 10         [ 7]  126     LD  A,#0x10             
   CB85 ED 79         [12]  127     OUT (C),A           
   CB87 0E 02         [ 7]  128     LD  C,#0x02         
   CB89                     129 CHKDAC2:    
   CB89 ED 78         [12]  130     IN  A,(C)   
   CB8B E6 04         [ 7]  131     AND #0x04           
   CB8D 28 FA         [12]  132     JR  Z,CHKDAC2       
   CB8F 0E 02         [ 7]  133     LD  C,#0x02         
   CB91 AF            [ 4]  134     XOR A             
   CB92 ED 79         [12]  135     OUT (C),A           
   CB94 0E 02         [ 7]  136     LD  C,#0x02         
   CB96                     137 CHKDAC3:    
   CB96 ED 78         [12]  138     IN  A,(C)          
   CB98 E6 04         [ 7]  139     AND #0x04           
   CB9A 20 FA         [12]  140     JR  NZ,CHKDAC3      
   CB9C C9            [10]  141     RET               
                            142     
   CB9D                     143 recvdata:
   CB9D C5            [11]  144     PUSH    BC           
   CB9E 0E 02         [ 7]  145     LD  C,#0x02             
   CBA0 06 C0         [ 7]  146     LD  B,#0xC0             
   CBA2 3E 20         [ 7]  147     LD  A,#0x20             
   CBA4 ED 79         [12]  148     OUT (C),A           
   CBA6 0E 02         [ 7]  149     LD  C,#0x02             
   CBA8                     150 CHKDAV0:    
   CBA8 ED 78         [12]  151     IN  A,(C)           
   CBAA E6 01         [ 7]  152     AND #0x01           
   CBAC 28 FA         [12]  153     JR  Z,CHKDAV0       
   CBAE 0E 02         [ 7]  154     LD  C,#0x02         
   CBB0 AF            [ 4]  155     XOR A               
   CBB1 ED 79         [12]  156     OUT (C),A           
   CBB3 0E 01         [ 7]  157     LD  C,#0x01             
   CBB5 ED 50         [12]  158     IN  D,(C)           
   CBB7 0E 02         [ 7]  159     LD  C,#0x02             
   CBB9 3E 40         [ 7]  160     LD  A,#0x40             
   CBBB ED 79         [12]  161     OUT (C),A         
   CBBD 0E 02         [ 7]  162     LD  C,#0x02             
   CBBF                     163 CHKDAV1:    
   CBBF ED 78         [12]  164     IN  A,(C)           
   CBC1 E6 01         [ 7]  165     AND #0x01           
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 4.
Hexadecimal [16-Bits]



   CBC3 20 FA         [12]  166     JR  NZ,CHKDAV1      
   CBC5 0E 02         [ 7]  167     LD  C,#0x02         
   CBC7 AF            [ 4]  168     XOR A               
   CBC8 ED 79         [12]  169     OUT (C),A           
   CBCA C1            [10]  170     POP BC              
   CBCB C9            [10]  171     RET           
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 5.
Hexadecimal [16-Bits]

Symbol Table

    .__.$$$.=  2710 L   |     .__.ABS.=  0000 G   |     .__.CPU.=  0000 L
    .__.H$L.=  0000 L   |     BDOS    =  D406     |     BIAS    =  9800 
    BIOS    =  CC00     |     BIOSL   =  0300     |     BOOTBAS =  FB23 
    CCP     =  CC00     |     CDISK   =  0004     |     CGINT   =  554A 
  5 CHKDAC2    CB89 R   |   5 CHKDAC3    CB96 R   |   5 CHKDAV0    CBA8 R
  5 CHKDAV1    CBBF R   |   5 CHKRFD1    CB72 R   |     CLR02   =  0AD5 
    CLS     =  1B42     |     CPM_BOOT=  CB00     |     DEPRT   =  07F3 
    DSKIX   =  F9DF     |     GRAPH   =  1B95     |     GSAVES  =  1BEA 
    INITSB  =  0050     |     IOBYTE  =  0003     |     MAIN    =  CC00 
    MEMSET  =  33E9     |     MSIZE   =  003A     |     NSECTS  =  0000 
  5 RDLOOPx    CB5B R   |     ROMPATCH=  FB29     |     SCRNS0  =  05F3 
    SDCOPY  =  0004     |     SDFORMAT=  0005     |     SDREAD  =  0002 
    SDSEND  =  0003     |     SDWRITE =  0001     |     SECTS   =  0006 
    SIZE    =  0300     |     TICONT  =  0036     |   5 _sd_load   CB38 R
  5 recvdata   CB9D R   |   5 sendcmd    CB66 R   |   5 senddata   CB6E R
  5 start      CB00 GR

ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 6.
Hexadecimal [16-Bits]

Area Table

   0 _CODE      size    0   flags    0
   1 HOME       size    0   flags    0
   2 XSEG       size    0   flags    0
   3 PSEG       size    0   flags    0
   4 _HEADER    size    0   flags    8
   5 _HEADER0   size   CC   flags    8

