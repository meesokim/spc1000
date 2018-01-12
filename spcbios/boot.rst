ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 1.
Hexadecimal [16-Bits]



                              1 ;==============================================================================
                              2 ;   Floppy booting code for SPC-1000
                              3 ;
                              4 ;           boot.s
                              5 ;                                   By meeso.kim
                              6 ;==============================================================================
                              7 
                              8     .module boot
                              9     .area   _CODE
                             10     .area  _HEADER  (ABS)
   CB00                      11     .org    CPM_BOOT
                             12     
                     003A    13 MSIZE   =   58
                     9800    14 BIAS    =   (MSIZE-20)*1024
                     CC00    15 CCP     =   0x3400+BIAS
                     E200    16 BIOS    =   CCP+0x1600
                     0300    17 BIOSL   =   0x0300      ;length of the bios
                     D406    18 BDOS    =   CCP+0x806   ;base of bdos
                             19 
                             20 ;BOOT   =   BIOS
                     1900    21 SIZE    =   BIOS+BIOSL-CCP  ;size of cp/m system
                     0032    22 SECTS   =   SIZE/128    ;# of sectors to load
                     002C    23 NSECTS  =   (BIOS-CCP)/128
                     F9DF    24 DSKIX   =   0x0fa00-33
                     1B42    25 CLS     =   0x1b42
                     0AD5    26 CLR02   =   0x0ad5
                     1BEA    27 GSAVES  =   0x1bea
                     0004    28 CDISK   =   0x0004      ;current disk number 0=A,...,15=P
                     0003    29 IOBYTE  =   0x0003      ;intel i/o byte
                     0036    30 TICONT  =   0x0036
                             31 
                     0001    32 SDWRITE     = 1
                     0002    33 SDREAD      = 2
                     0003    34 SDSEND      = 3
                     0004    35 SDCOPY      = 4
                     0005    36 SDFORMAT    = 5
                             37             
                     CB00    38 CPM_BOOT = 0xcb00   
                             39 
   CB00                      40 start::
                             41 ;   .ascii "SYS"
   CB00 DD 21 DF F9   [14]   42     ld  ix, #DSKIX
   CB04 DD F9         [10]   43     ld  sp, ix
   CB06 CD D5 0A      [17]   44 	call CLR02
   CB09 CD EA 1B      [17]   45     call GSAVES
   CB0C 01 02 00      [10]   46     ld  bc, #0x002
   CB0F 26 0F         [ 7]   47     ld  h,  #15
   CB11 11 00 CC      [10]   48     ld  de, #CCP
   CB14 CD 3B CB      [17]   49     call _sd_load
   CB17 01 01 01      [10]   50     ld  bc, #0x101
   CB1A 26 08         [ 7]   51     ld  h,  #8
   CB1C 11 00 DB      [10]   52     ld  de, #CCP+0xf00
   CB1F CD 3B CB      [17]   53     call _sd_load
   CB22 01 09 01      [10]   54     ld  bc, #0x109
   CB25 26 08         [ 7]   55     ld  h,  #8
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 2.
Hexadecimal [16-Bits]



   CB27 11 00 E2      [10]   56     ld  de, #BIOS
   CB2A CD 3B CB      [17]   57     call _sd_load
   CB2D 01 01 02      [10]   58     ld  bc, #0x201
   CB30 26 07         [ 7]   59     ld  h,  #7
   CB32 11 00 EA      [10]   60     ld  de, #BIOS+0x800
   CB35 CD 3B CB      [17]   61     call _sd_load
   CB38 C3 00 E2      [10]   62     jp BIOS
                             63     
   CB3B                      64 _sd_load:
   CB3B E5            [11]   65     push hl
   CB3C D5            [11]   66     push de
   CB3D C5            [11]   67     push bc
   CB3E 16 02         [ 7]   68     ld  d, #SDREAD
   CB40 CD 69 CB      [17]   69     call sendcmd
   CB43 54            [ 4]   70     ld  d, h
   CB44 CD 71 CB      [17]   71     call senddata
   CB47 16 00         [ 7]   72     ld  d, #0
   CB49 CD 71 CB      [17]   73     call senddata
   CB4C E1            [10]   74     pop hl
   CB4D 54            [ 4]   75     ld  d, h
   CB4E CD 71 CB      [17]   76     call senddata
   CB51 55            [ 4]   77     ld  d, l
   CB52 CD 71 CB      [17]   78     call senddata
   CB55 16 03         [ 7]   79     ld  d, #SDSEND
   CB57 CD 69 CB      [17]   80     call sendcmd
   CB5A E1            [10]   81     pop hl
   CB5B C1            [10]   82     pop bc
   CB5C 0E 00         [ 7]   83     ld  c,#0
   CB5E                      84 RDLOOPx:
   CB5E CD A0 CB      [17]   85     call recvdata
   CB61 72            [ 7]   86     ld (hl), d
   CB62 23            [ 6]   87     inc hl
   CB63 0B            [ 6]   88     dec bc
   CB64 78            [ 4]   89     ld  a, b
   CB65 B1            [ 4]   90     or  c
   CB66 20 F6         [12]   91     jr nz, RDLOOPx
   CB68 C9            [10]   92     ret 
                             93 
   CB69                      94 sendcmd:
   CB69 06 C0         [ 7]   95     LD  B,#0xC0             
   CB6B 0E 02         [ 7]   96     LD  C,#0x02             
   CB6D 3E 80         [ 7]   97     LD  A,#0x80             
   CB6F ED 79         [12]   98     OUT (C),A           
   CB71                      99 senddata:   
   CB71 06 C0         [ 7]  100     LD  B,#0xC0             
   CB73 0E 02         [ 7]  101     LD  C,#0x02             
   CB75                     102 CHKRFD1:    
   CB75 ED 78         [12]  103     IN  A,(C)           
   CB77 E6 02         [ 7]  104     AND #0x02           
   CB79 28 FA         [12]  105     JR  Z,CHKRFD1       
   CB7B 0E 02         [ 7]  106     LD  C,#0x02             
   CB7D AF            [ 4]  107     XOR A               
   CB7E ED 79         [12]  108     OUT (C),A           
   CB80 0E 00         [ 7]  109     LD  C,#0x00             
   CB82 ED 51         [12]  110     OUT (C),D           
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 3.
Hexadecimal [16-Bits]



   CB84 0E 02         [ 7]  111     LD  C,#0x02             
   CB86 3E 10         [ 7]  112     LD  A,#0x10             
   CB88 ED 79         [12]  113     OUT (C),A           
   CB8A 0E 02         [ 7]  114     LD  C,#0x02         
   CB8C                     115 CHKDAC2:    
   CB8C ED 78         [12]  116     IN  A,(C)   
   CB8E E6 04         [ 7]  117     AND #0x04           
   CB90 28 FA         [12]  118     JR  Z,CHKDAC2       
   CB92 0E 02         [ 7]  119     LD  C,#0x02         
   CB94 AF            [ 4]  120     XOR A             
   CB95 ED 79         [12]  121     OUT (C),A           
   CB97 0E 02         [ 7]  122     LD  C,#0x02         
   CB99                     123 CHKDAC3:    
   CB99 ED 78         [12]  124     IN  A,(C)          
   CB9B E6 04         [ 7]  125     AND #0x04           
   CB9D 20 FA         [12]  126     JR  NZ,CHKDAC3      
   CB9F C9            [10]  127     RET               
                            128     
   CBA0                     129 recvdata:
   CBA0 C5            [11]  130     PUSH    BC           
   CBA1 0E 02         [ 7]  131     LD  C,#0x02             
   CBA3 06 C0         [ 7]  132     LD  B,#0xC0             
   CBA5 3E 20         [ 7]  133     LD  A,#0x20             
   CBA7 ED 79         [12]  134     OUT (C),A           
   CBA9 0E 02         [ 7]  135     LD  C,#0x02             
   CBAB                     136 CHKDAV0:    
   CBAB ED 78         [12]  137     IN  A,(C)           
   CBAD E6 01         [ 7]  138     AND #0x01           
   CBAF 28 FA         [12]  139     JR  Z,CHKDAV0       
   CBB1 0E 02         [ 7]  140     LD  C,#0x02         
   CBB3 AF            [ 4]  141     XOR A               
   CBB4 ED 79         [12]  142     OUT (C),A           
   CBB6 0E 01         [ 7]  143     LD  C,#0x01             
   CBB8 ED 50         [12]  144     IN  D,(C)           
   CBBA 0E 02         [ 7]  145     LD  C,#0x02             
   CBBC 3E 40         [ 7]  146     LD  A,#0x40             
   CBBE ED 79         [12]  147     OUT (C),A         
   CBC0 0E 02         [ 7]  148     LD  C,#0x02             
   CBC2                     149 CHKDAV1:    
   CBC2 ED 78         [12]  150     IN  A,(C)           
   CBC4 E6 01         [ 7]  151     AND #0x01           
   CBC6 20 FA         [12]  152     JR  NZ,CHKDAV1      
   CBC8 0E 02         [ 7]  153     LD  C,#0x02         
   CBCA AF            [ 4]  154     XOR A               
   CBCB ED 79         [12]  155     OUT (C),A           
   CBCD C1            [10]  156     POP BC              
   CBCE C9            [10]  157     RET           
                            158 
   E200                     159     .org BIOS
                            160     
   E200 C3 12 E6      [10]  161     JP BOOT     ;cold start
   E203 C3 30 E6      [10]  162 WBOOTE: JP  WBOOT       ;warm start
   E206 C3 9B E6      [10]  163     JP CONST        ;console status
   E209 C3 A3 E6      [10]  164     JP CONIN        ;console character in
   E20C C3 A7 E6      [10]  165     JP CONOUT       ;console character out
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 4.
Hexadecimal [16-Bits]



   E20F C3 AE E6      [10]  166     JP LIST     ;list character out
   E212 C3 AF E6      [10]  167     JP PUNCHS       ;punch character out
   E215 C3 B1 E6      [10]  168     JP READER       ;reader character in
   E218 C3 B2 E6      [10]  169     JP HOME     ;move head to home position
   E21B C3 D2 E6      [10]  170     JP SELDSK       ;select disk
   E21E C3 DB E6      [10]  171     JP SETTRK       ;set track number
   E221 C3 EA E6      [10]  172     JP SETSEC       ;set sector number
   E224 C3 02 E7      [10]  173     JP SETDMA       ;set dma address
   E227 C3 19 E7      [10]  174     JP READ     ;read disk
   E22A C3 4A E7      [10]  175     JP WRITE        ;write disk
   E22D C3 07 E7      [10]  176     JP LISTST       ;return list status
   E230 C3 08 E7      [10]  177     JP SECTRAN      ;sector translate
                            178 
   E233                     179 DPBASE: 
   E233 00 00 00 00         180     .dw 0,0
   E237 00 00 00 00         181     .dw 0,0
   E23B C5 E2 6D E2         182     .dw DIRBF,DPBLK
   E23F BF E5 45 E3         183     .dw CHK00,ALL00
                            184 ;   disk parameter header for disk 01
   E243 00 00 00 00         185     .dw 0,0
   E247 00 00 00 00         186     .dw 0,0
   E24B C5 E2 6D E2         187     .dw DIRBF,DPBLK
   E24F CF E5 64 E3         188     .dw CHK01,ALL01
                            189 ;
                            190 ;   sector translate vector for the IBM 8" SD disks
                            191 ;
   E253                     192 TRANS:  
   E253 01 07 0D 13         193     .db 1,7,13,19   ;sectors 1,2,3,4
   E257 19 05 0B 11         194     .db 25,5,11,17  ;sectors 5,6,7,8
   E25B 17 03 09 0F         195     .db 23,3,9,15   ;sectors 9,10,11,12
   E25F 15 02 08 0E         196     .db 21,2,8,14   ;sectors 13,14,15,16
   E263 14 1A 06 0C         197     .db 20,26,6,12  ;sectors 17,18,19,20
   E267 12 18 04 0A         198     .db 18,24,4,10  ;sectors 21,22,23,24
   E26B 10 16               199     .db 16,22       ;sectors 25,26
                            200 ;
                            201 ;   disk parameter block, SD725 (from TF-20)
                            202 ;
   E26D                     203 DPBLK:  
   E26D 28 00               204     .dw 40      ;sectors per track
   E26F 04                  205     .db 4       ;block shift factor
   E270 0F                  206     .db 15      ;block mask
   E271 01                  207     .db 1       ;extent mask
   E272 99 00               208     .dw 153     ;disk size-1
   E274 7F 00               209     .dw 127     ;directory max
   E276 C0                  210     .db 0xc0    ;alloc 0
   E277 00                  211     .db 0       ;alloc 1
   E278 20 00               212     .dw 32      ;check size
   E27A 03 00               213     .dw 3       ;track offset
                            214 
                            215 ;   messages
                            216 ;
   E27C                     217 SIGNON: 
   E27C 35 38 4B 20 43 50   218     .ascii  '58K CP/M Version 2.2'
        2F 4D 20 56 65 72
        73 69 6F 6E 20 32
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 5.
Hexadecimal [16-Bits]



        2E 32
   E290 0D 0A               219     .db 13,10
   E292 28 66 6F 72 20 53   220     .ascii  '(for SPC-1000 with SD-725)'
        50 43 2D 31 30 30
        30 20 77 69 74 68
        20 53 44 2D 37 32
        35 29
   E2AC 0D 0A 00            221     .db 13,10,0
                            222 ;
   E2AF                     223 LDERR:  
   E2AF 42 49 4F 53 3A 20   224     .ascii  'BIOS: error booting'
        65 72 72 6F 72 20
        62 6F 6F 74 69 6E
        67
   E2C2 0D 0A 00            225     .db 13,10,0 
                            226 
   E2C5                     227 DIRBF:  .ds 128     ;scratch directory area
   E345                     228 ALL00:  .ds 31      ;allocation vector 0
   E364                     229 ALL01:  .ds 31      ;allocation vector 1
   E383                     230 ALL02:  .ds 31      ;allocation vector 2
   E3A2                     231 ALL03:  .ds 31      ;allocation vector 3
   E3C1                     232 ALLHD1: .ds 255     ;allocation vector harddisk 1
   E4C0                     233 ALLHD2: .ds 255     ;allocation vector harddisk 2
   E5BF                     234 CHK00:  .ds 16      ;check vector 0
   E5CF                     235 CHK01:  .ds 16      ;check vector 1
   E5DF                     236 CHK02:  .ds 16      ;check vector 2
   E5EF                     237 CHK03:  .ds 16      ;check vector 3
   E5FF                     238 CHKHD1: .ds 0       ;check vector harddisk 1
   E5FF                     239 CHKHD2: .ds 0       ;check vector harddisk 2
                            240 
                            241 
   E5FF 01                  242 FDDN:   .db 1
   E600 00                  243 FDDD:   .db 0
   E601 00                  244 FDDT:   .db 0
   E602 00                  245 FDDS:   .db 0
   E603 00 00               246 DATADDR:.dw 0
   E605 01                  247 CHGFLG: .db 1
   E606 FF                  248 PRESEC: .db 0xff    
                            249 
   E607 7E            [ 7]  250 PRTMSG: LD  A,(HL)
   E608 B7            [ 4]  251     OR  A
   E609 C8            [11]  252     RET Z
   E60A 4F            [ 4]  253     LD  C,A
   E60B CD A7 E6      [17]  254     CALL    CONOUT
   E60E 23            [ 6]  255     INC HL
   E60F C3 07 E6      [10]  256     JP  PRTMSG
                            257 ;
                            258 ;   individual subroutines to perform each function
                            259 ;   simplest case is to just perform parameter initialization
                            260 ;
   E612                     261 BOOT:   
   E612 31 80 00      [10]  262     LD  SP,#0x80        ;use space below buffer for stack
   E615 AF            [ 4]  263     XOR A
   E616 01 00 60      [10]  264     LD  BC, #0x6000
   E619 ED 79         [12]  265     OUT (C), A
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 6.
Hexadecimal [16-Bits]



   E61B 06 00         [ 7]  266 	LD  B,#0
   E61D CD 7E ED      [17]  267 	CALL SCRNBC	
   E620 21 7C E2      [10]  268     LD  HL,#SIGNON  ;print message
   E623 CD 07 E6      [17]  269     CALL    PRTMSG
   E626 AF            [ 4]  270     XOR A       ;zero in the accum
   E627 32 03 00      [13]  271     LD  (#IOBYTE),A ;clear the iobyte
   E62A 32 04 00      [13]  272     LD  (#CDISK),A  ;select disk zero
   E62D C3 7A E6      [10]  273     JP  GOCPM       ;initialize and go to cp/m  
   E630                     274 WBOOT:      ;warm start
   E630 31 80 00      [10]  275     LD  SP,#0x80        ;use space below buffer for stack
   E633 0E 00         [ 7]  276     LD  C,#0        ;select disk 0
   E635 CD D2 E6      [17]  277     CALL    SELDSK
   E638 CD B2 E6      [17]  278     CALL    HOME        ;go to track 00
                            279 ;
   E63B 06 2C         [ 7]  280     LD  B,#NSECTS   ;b counts # of sectors to load
   E63D 0E 00         [ 7]  281     LD  C,#0        ;c has the current track number
   E63F 16 04         [ 7]  282     LD  D,#4        ;d has the next sector to read
                            283 ;   note that we begin by reading track 0, sector 2 since sector 1
                            284 ;   contains the cold start loader, which is skipped in a warm start
   E641 21 00 CC      [10]  285     LD  HL,#CCP     ;base of cp/m (initial load point)
   E644                     286 LOAD1:              ;load one more sector
   E644 C5            [11]  287     PUSH    BC      ;save sector count, current track
   E645 D5            [11]  288     PUSH    DE      ;save next sector to read
   E646 E5            [11]  289     PUSH    HL      ;save dma address
   E647 4A            [ 4]  290     LD  C,D     ;get sector address to register c
   E648 CD EA E6      [17]  291     CALL    SETSEC      ;set sector address from register c
   E64B C1            [10]  292     POP BC      ;recall dma address to b,c
   E64C C5            [11]  293     PUSH    BC      ;replace on stack for later recall
   E64D CD 02 E7      [17]  294     CALL    SETDMA      ;set dma address from b,c
                            295 ;   drive set to 0, track set, sector set, dma address set
   E650 CD 19 E7      [17]  296     CALL    READ
   E653 B7            [ 4]  297     OR  A       ;any errors?
   E654 CA 5F E6      [10]  298     JP  Z,LOAD2     ;no, continue
   E657 21 AF E2      [10]  299     LD  HL,#LDERR   ;error, print message
   E65A CD 07 E6      [17]  300     CALL    PRTMSG
   E65D F3            [ 4]  301     DI          ;and halt the machine
   E65E 76            [ 4]  302     HALT
                            303 ;   no error, move to next sector
   E65F E1            [10]  304 LOAD2:  POP HL      ;recall dma address
   E660 11 80 00      [10]  305     LD  DE,#128     ;dma=dma+128
   E663 19            [11]  306     ADD HL,DE       ;new dma address is in h,l
   E664 D1            [10]  307     POP DE      ;recall sector address
   E665 C1            [10]  308     POP BC      ;recall number of sectors remaining,
                            309                 ;and current trk
   E666 05            [ 4]  310     DEC B       ;sectors=sectors-1
   E667 CA 7A E6      [10]  311     JP  Z,GOCPM     ;transfer to cp/m if all have been loaded
                            312 ;   more sectors remain to load, check for track change
   E66A 14            [ 4]  313     INC D
   E66B 7A            [ 4]  314     LD  A,D     ;sector=27?, if so, change tracks
   E66C FE 1F         [ 7]  315     CP  #31
   E66E DA 44 E6      [10]  316     JP  C,LOAD1     ;carry generated if sector<27
                            317 ;   end of current track, go to next track
   E671 16 01         [ 7]  318     LD  D,#1        ;begin with first sector of next track
   E673 0C            [ 4]  319     INC C       ;track=track+1
                            320 ;   save register state, and change tracks
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 7.
Hexadecimal [16-Bits]



   E674 CD DB E6      [17]  321     CALL    SETTRK      ;track address set from register c
   E677 C3 44 E6      [10]  322     JP  LOAD1       ;for another sector
                            323 ;   end of load operation, set parameters and go to cp/m
   E67A                     324 GOCPM:
   E67A 3E C3         [ 7]  325     LD  A,#0x0C3        ;c3 is a jmp instruction
   E67C 32 00 00      [13]  326     LD  (#0),A      ;for jmp to wboot
   E67F 21 03 E2      [10]  327     LD  HL,#WBOOTE  ;wboot entry point
   E682 22 01 00      [16]  328     LD  (#1),HL     ;set address field for jmp at 0
                            329 ;
   E685 32 05 00      [13]  330     LD  (#5),A      ;for jmp to bdos
   E688 21 06 D4      [10]  331     LD  HL,#BDOS        ;bdos entry point
   E68B 22 06 00      [16]  332     LD  (#6),HL     ;address field of jump at 5 to bdos
                            333 ;
   E68E 01 80 00      [10]  334     LD  BC,#0x80        ;default dma address is 80h
   E691 CD 02 E7      [17]  335     CALL    SETDMA
                            336 ;
   E694 3A 04 00      [13]  337     LD  A,(#CDISK)  ;get current disk number
   E697 4F            [ 4]  338     LD  C,A     ;send to the ccp
   E698 C3 00 CC      [10]  339     JP  CCP     ;go to cp/m for further processing  
   E69B                     340 CONST:      ;console status
   E69B 3A C6 EE      [13]  341     LD A, (#KEYBUF)
   E69E B7            [ 4]  342     OR A
   E69F C0            [11]  343     RET NZ
   E6A0 3E FF         [ 7]  344     LD A, #0xff
   E6A2 C9            [10]  345     RET
   E6A3                     346 CONIN:      ;console character in
                            347 ;   IN  A,(CONDAT)  ;get character from console
   E6A3 CD 84 E7      [17]  348     CALL ASCGET1        
   E6A6 C9            [10]  349     RET
   E6A7                     350 CONOUT:     ;console character out
   E6A7 79            [ 4]  351     LD  A,C     ;get to accumulator
   E6A8                     352 ASCPRT:
   E6A8 E5            [11]  353     PUSH HL
   E6A9 CD 06 EA      [17]  354     CALL ACCPRT1
   E6AC E1            [10]  355     POP  HL
   E6AD C9            [10]  356     RET
   E6AE                     357 LIST:       ;list character out
   E6AE 79            [ 4]  358     LD  A,C     ;character to register a
                            359     
   E6AF                     360 PUNCHS:     ;punch character out
   E6AF 79            [ 4]  361     LD  A,C     ;character to register a
                            362 ;   OUT (AUXDAT),A
   E6B0 C9            [10]  363     RET
   E6B1                     364 READER:     ;reader character in
   E6B1 C9            [10]  365     RET
   E6B2                     366 HOME:       ;move head to home position
   E6B2 0E 00         [ 7]  367     LD  C,#0        ;select track 0
   E6B4 C3 DB E6      [10]  368     JP  SETTRK      ;we will move to 00 on first read/write
   E6B7                     369 SELFD:
   E6B7 4F            [ 4]  370     LD  C,A
   E6B8 3A 00 E6      [13]  371     LD  A,(#FDDD)
   E6BB B9            [ 4]  372     CP  C
   E6BC 28 05         [12]  373     JR  Z,SELFD$ 
   E6BE 21 05 E6      [10]  374     LD HL,#CHGFLG
   E6C1 CB D6         [15]  375     SET #2,(HL)
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 8.
Hexadecimal [16-Bits]



   E6C3                     376 SELFD$: 
   E6C3 26 00         [ 7]  377     LD H,#0
   E6C5 32 00 E6      [13]  378     LD (#FDDD),A
   E6C8 6F            [ 4]  379     LD  L,A     ;L=disk number 0,1,2,3
   E6C9 29            [11]  380     ADD HL,HL       ;*2
   E6CA 29            [11]  381     ADD HL,HL       ;*4
   E6CB 29            [11]  382     ADD HL,HL       ;*8
   E6CC 29            [11]  383     ADD HL,HL       ;*16 (size of each header)
   E6CD 11 33 E2      [10]  384     LD  DE,#DPBASE
   E6D0 19            [11]  385     ADD HL,DE       ;HL=.dpbase(diskno*16)
   E6D1 C9            [10]  386     RET
   E6D2                     387 SELDSK:     ;select disk
   E6D2 21 00 00      [10]  388     LD  HL,#0   ;error return code
   E6D5 79            [ 4]  389     LD  A,C
   E6D6 FE 02         [ 7]  390     CP  #2      ;FD drive 0-1?
   E6D8 38 DD         [12]  391     JR  C,#SELFD        ;go
   E6DA C9            [10]  392     RET 
   E6DB                     393 SETTRK:     ;set track number
   E6DB 3A 01 E6      [13]  394     LD A,(#FDDT)
   E6DE B9            [ 4]  395     CP C
   E6DF C8            [11]  396     RET Z
   E6E0 21 05 E6      [10]  397     LD HL,#CHGFLG
   E6E3 CB CE         [15]  398     SET 1,(HL)
   E6E5 79            [ 4]  399     LD  A,C
   E6E6 32 01 E6      [13]  400     LD (#FDDT),A
   E6E9 C9            [10]  401     RET
   E6EA                     402 SETSEC:     ;set sector number
   E6EA 0D            [ 4]  403     DEC C
   E6EB 41            [ 4]  404     LD B,C
   E6EC AF            [ 4]  405     XOR A
   E6ED 3A 02 E6      [13]  406     LD A,(#FDDS)
   E6F0 CB 1F         [ 8]  407     RR A
   E6F2 B7            [ 4]  408     OR A
   E6F3 CB 19         [ 8]  409     RR C
   E6F5 B9            [ 4]  410     CP C
   E6F6 28 05         [12]  411     JR Z, SETSEC0
   E6F8 21 05 E6      [10]  412     LD HL,#CHGFLG
   E6FB CB C6         [15]  413     SET #0,(HL)
   E6FD                     414 SETSEC0:    
   E6FD 78            [ 4]  415     LD  A,B
   E6FE 32 02 E6      [13]  416     LD (#FDDS),A
   E701 C9            [10]  417     RET
   E702                     418 SETDMA:     ;set dma address
   E702 ED 43 03 E6   [20]  419     LD (#DATADDR), BC
   E706 C9            [10]  420     RET
   E707                     421 LISTST:     ;return list status
   E707 C9            [10]  422     RET
   E708                     423 SECTRAN:    ;sector translate
   E708 7A            [ 4]  424     LD  A,D     ;do we have a translation table?
   E709 B3            [ 4]  425     OR  E
   E70A C2 13 E7      [10]  426     JP  NZ,SECT1    ;yes, translate
   E70D 69            [ 4]  427     LD  L,C     ;no, return untranslated
   E70E 60            [ 4]  428     LD  H,B     ;in HL
   E70F 2C            [ 4]  429     INC L       ;sector no. start with 1
   E710 C0            [11]  430     RET NZ
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 9.
Hexadecimal [16-Bits]



   E711 24            [ 4]  431     INC H
   E712 C9            [10]  432     RET
   E713 EB            [ 4]  433 SECT1:  EX  DE,HL       ;HL=.trans
   E714 09            [11]  434     ADD HL,BC       ;HL=.trans(sector)
   E715 6E            [ 7]  435     LD  L,(HL)      ;L = trans(sector)
   E716 26 00         [ 7]  436     LD  H,#0        ;HL= trans(sector)
   E718 C9            [10]  437     RET         ;with data in HL
                            438 
   E719                     439 READ:       ;read disk
   E719 16 02         [ 7]  440     LD  D, #SDREAD                       ;e3aa  16 0e          355    369 
   E71B CD 69 CB      [17]  441     CALL    sendcmd                     ;e3ac  cd 01 e4       356    370 
   E71E 21 FF E5      [10]  442     LD  HL,#FDDN                        ;e3af  21 67 e4       357    371 
   E721 56            [ 7]  443     LD  D,(HL)                          ;e3b2  56             358    372 
   E722 CD 71 CB      [17]  444     CALL    senddata                     ;e3b3  cd 09 e4       359    373 
   E725 23            [ 6]  445     INC HL                              ;e3b6  23             360    374 
   E726 56            [ 7]  446     LD  D,(HL)                          ;e3b7  56             361    375 
   E727 CD 71 CB      [17]  447     CALL    senddata                     ;e3b8  cd 09 e4       362    376 
   E72A 23            [ 6]  448     INC HL                              ;e3bb  23             363    377 
   E72B 56            [ 7]  449     LD  D,(HL)                          ;e3bc  56             364    378 
   E72C CD 71 CB      [17]  450     CALL    senddata                     ;e3bd  cd 09 e4       365    379 
   E72F 23            [ 6]  451     INC HL                              ;e3c0  23             366    380 
   E730 56            [ 7]  452     LD  D,(HL)                          ;e3c1  56             367    381 
   E731 CD 71 CB      [17]  453     CALL    senddata                     ;e3c3  cd 09 e4       369    383 
   E734 16 03         [ 7]  454     ld d, #SDSEND
   E736 CD 69 CB      [17]  455     call sendcmd
   E739 2A 03 E6      [16]  456     ld hl, (#DATADDR)
   E73C 01 00 01      [10]  457     ld bc, #0x100
   E73F                     458 RDLOOP:
   E73F CD A0 CB      [17]  459     call recvdata
   E742 72            [ 7]  460     ld (hl), d
   E743 23            [ 6]  461     inc hl
   E744 0B            [ 6]  462     dec bc
   E745 78            [ 4]  463     ld  a, b
   E746 B1            [ 4]  464     or  c
   E747 20 F6         [12]  465     jr nz, RDLOOP
   E749 C9            [10]  466     ret
                            467     
   E74A                     468 WRITE:      ;write disk 
   E74A 16 01         [ 7]  469     LD  D, #SDWRITE                       ;e3aa  16 0e          355    369 
   E74C CD 69 CB      [17]  470     CALL    sendcmd                     ;e3ac  cd 01 e4       356    370 
   E74F 21 FF E5      [10]  471     LD  HL,#FDDN                        ;e3af  21 67 e4       357    371 
   E752 56            [ 7]  472     LD  D,(HL)                          ;e3b2  56             358    372 
   E753 CD 71 CB      [17]  473     CALL    senddata                     ;e3b3  cd 09 e4       359    373 
   E756 23            [ 6]  474     INC HL                              ;e3b6  23             360    374 
   E757 56            [ 7]  475     LD  D,(HL)                          ;e3b7  56             361    375 
   E758 CD 71 CB      [17]  476     CALL    senddata                     ;e3b8  cd 09 e4       362    376 
   E75B 23            [ 6]  477     INC HL                              ;e3bb  23             363    377 
   E75C 56            [ 7]  478     LD  D,(HL)                          ;e3bc  56             364    378 
   E75D CD 71 CB      [17]  479     CALL    senddata                     ;e3bd  cd 09 e4       365    379 
   E760 23            [ 6]  480     INC HL                              ;e3c0  23             366    380 
   E761 56            [ 7]  481     LD  D,(HL)                          ;e3c1  56             367    381 
   E762 CD 71 CB      [17]  482     CALL    senddata                     ;e3c3  cd 09 e4       369    383 
   E765 16 03         [ 7]  483     ld d, #SDSEND
   E767 CD 69 CB      [17]  484     call sendcmd
   E76A 2A 03 E6      [16]  485     ld hl, (#DATADDR)
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 10.
Hexadecimal [16-Bits]



   E76D 01 00 01      [10]  486     ld bc, #0x100
   E770                     487 WRLOOP: 
   E770 56            [ 7]  488     ld d,(hl)
   E771 C5            [11]  489     push bc
   E772 CD 71 CB      [17]  490     call senddata
   E775 C1            [10]  491     pop bc
   E776 23            [ 6]  492     inc hl
   E777 0B            [ 6]  493     dec bc
   E778 78            [ 4]  494     ld a, b
   E779 B1            [ 4]  495     or c
   E77A 20 F4         [12]  496     jr nz, WRLOOP
   E77C C9            [10]  497     ret 
                            498 ;
                            499 ; KEY GET
   E77D 3E 80         [ 7]  500 BRKEY:  LD  A,#0x80
   E77F DB 00         [11]  501     IN  A,(0)
   E781 E6 12         [ 7]  502     AND #0x12
   E783 C9            [10]  503     RET 
   E784                     504 ASCGET1: 
   E784 C5            [11]  505     PUSH    BC
   E785 D5            [11]  506     PUSH    DE
   E786 E5            [11]  507     PUSH    HL
   E787 2A 64 EE      [16]  508     LD  HL,(POINT1)
   E78A 7D            [ 4]  509     LD  A,L
   E78B BC            [ 4]  510     CP  H
   E78C 28 12         [12]  511     JR  Z,BUFSET
   E78E 3C            [ 4]  512 BUFCEK: INC A
   E78F E6 3F         [ 7]  513     AND #0x3F
   E791 32 64 EE      [13]  514     LD  (POINT1),A
   E794 6F            [ 4]  515     LD  L,A
   E795 26 00         [ 7]  516     LD  H,#0
   E797 11 66 EE      [10]  517     LD  DE,#INKYBF
   E79A 19            [11]  518     ADD HL,DE
   E79B 7E            [ 7]  519     LD  A,(HL)
   E79C E1            [10]  520 GETRET: POP HL
   E79D D1            [10]  521     POP DE
   E79E C1            [10]  522     POP BC
   E79F C9            [10]  523     RET
                            524 ;
   E7A0 3E                  525 BUFSET: .db 0x3E        ; LD A,
   E7A1 01                  526 FLASHF: .db 0x01        ;
   E7A2 B7            [ 4]  527     OR  A
   E7A3 20 24         [12]  528     JR  NZ,FLGET
   E7A5 CD 9C E8      [17]  529     CALL    KEYSEN
   E7A8 CD 7D E7      [17]  530     CALL    BRKEY
   E7AB 20 07         [12]  531     JR  NZ,KEYGT
   E7AD CD 03 E9      [17]  532 BRKGTR: CALL    NEWFLG
   E7B0 3E 03         [ 7]  533     LD  A,#3
   E7B2 18 E8         [12]  534     JR  GETRET
   E7B4 CD 9C E8      [17]  535 KEYGT:  CALL    KEYSEN
   E7B7 28 0A         [12]  536     JR  Z,ZERORT
   E7B9 CD DD E8      [17]  537     CALL    NEWCEK
   E7BC 20 29         [12]  538     JR  NZ,NEWKYS
   E7BE CD F3 E8      [17]  539     CALL    SAMECK
   E7C1 28 16         [12]  540     JR  Z,REPETS
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 11.
Hexadecimal [16-Bits]



   E7C3 CD 03 E9      [17]  541 ZERORT: CALL    NEWFLG
   E7C6 AF            [ 4]  542 ZEROR2: XOR A
   E7C7 18 D3         [12]  543     JR  GETRET
   E7C9 CD 5E E8      [17]  544 FLGET:  CALL    FLSHGT
   E7CC 4F            [ 4]  545     LD  C,A
   E7CD 3E 80         [ 7]  546     LD  A,#0x80
   E7CF DB 00         [11]  547     IN  A,(0)
   E7D1 E6 12         [ 7]  548     AND #0x12
   E7D3 28 D8         [12]  549     JR  Z,BRKGTR
   E7D5 79            [ 4]  550     LD  A,C
   E7D6 B7            [ 4]  551     OR  A
   E7D7 28 0E         [12]  552     JR  Z,NEWKYS
   E7D9 CD 03 E9      [17]  553 REPETS: CALL    NEWFLG
   E7DC 21                  554     .db 0x21        ; LD HL,(KEYBFA)
   E7DD FE FF               555 KEYBFA: .dw 0xFFFE      ;
   E7DF 3E                  556     .db 0x3E        ; LD A,(KEYBFD)
   E7E0 00                  557 KEYBFD: .db 0
   E7E1 77            [ 7]  558     LD  (HL),A
   E7E2 21 1E 00      [10]  559     LD  HL,#30
   E7E5 18 06         [12]  560     JR  KEYST2
   E7E7 CD 03 E9      [17]  561 NEWKYS: CALL    NEWFLG
   E7EA 21                  562     .db 0x21        ; LD HL,300
   E7EB C8 00               563 REPTSW: .dw 200     ; auto repeat key duration
   E7ED 22 79 E8      [16]  564 KEYST2: LD  (REPETF),HL
   E7F0 21 58 EE      [10]  565     LD  HL,#NEWBUF
   E7F3 16 00         [ 7]  566     LD  D,#0
   E7F5 0E 09         [ 7]  567     LD  C,#9
   E7F7 06 08         [ 7]  568 SETLP1: LD  B,#8
   E7F9 7E            [ 7]  569     LD  A,(HL)
   E7FA 2F            [ 4]  570     CPL
   E7FB B7            [ 4]  571     OR  A
   E7FC 28 09         [12]  572     JR  Z,SETPL3
   E7FE 0F            [ 4]  573 SETPL2: RRCA
   E7FF DA 57 E9      [10]  574     JP  C,KYBFST
   E802 14            [ 4]  575     INC D
   E803 10 F9         [13]  576     DJNZ    SETPL2
   E805 18 04         [12]  577     JR  SETPL4
   E807 7A            [ 4]  578 SETPL3: LD  A,D
   E808 C6 08         [ 7]  579     ADD A,#8
   E80A 57            [ 4]  580     LD  D,A
   E80B 23            [ 6]  581 SETPL4: INC HL
   E80C 0D            [ 4]  582     DEC C
   E80D 20 E8         [12]  583     JR  NZ,SETLP1
   E80F 2A 64 EE      [16]  584 KYBFSR: LD  HL,(POINT1)
   E812 7D            [ 4]  585     LD  A,L
   E813 BC            [ 4]  586     CP  H
   E814 CA C6 E7      [10]  587     JP  Z,ZEROR2
   E817 C3 8E E7      [10]  588     JP  BUFCEK
                            589 ;
   E81A CD EF E9      [17]  590 FLASHS: CALL    ADRCAL
   E81D CB D8         [ 8]  591     SET 3,B
   E81F ED 43 37 E8   [20]  592     LD  (FLASHA),BC
   E823 ED 78         [12]  593     IN  A,(C)
   E825 32 3A E8      [13]  594     LD  (FLASHD),A
   E828 F6 01         [ 7]  595     OR  #1
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 12.
Hexadecimal [16-Bits]



   E82A ED 79         [12]  596     OUT (C),A
   E82C 32 4C E8      [13]  597     LD  (FLASHO),A
   E82F 3A 36 00      [13]  598     LD  A,(TICONT)
   E832 32 42 E8      [13]  599     LD  (FLASHC+1),A    ;★"+1"이 빠져있었음.
   E835 C9            [10]  600     RET         ;
                            601 ;
                            602 ;
   E836 01                  603 FLASHG: .db 1
   E837 00 00               604 FLASHA: .dw 0
   E839 3E                  605     .db 0x3E        ; LD A,
   E83A 00                  606 FLASHD: .db 0       ;
   E83B E6 FE         [ 7]  607     AND #0xFE
   E83D 5F            [ 4]  608     LD  E,A
   E83E 3A 36 00      [13]  609     LD  A,(TICONT)
   E841 D6 00         [ 7]  610 FLASHC: SUB #0          ; cursor blink time
   E843 E6 10         [ 7]  611     AND #0x10
   E845 3E 01         [ 7]  612     LD  A,#1
   E847 28 01         [12]  613     JR  Z,L0D28
   E849 AF            [ 4]  614     XOR A
   E84A B3            [ 4]  615 L0D28:  OR  E
   E84B FE                  616     .db 0xFE        ;CP
   E84C 00                  617 FLASHO: .db 0
   E84D C8            [11]  618     RET Z
   E84E ED 79         [12]  619     OUT (C),A
   E850 32 4C E8      [13]  620     LD  (FLASHO),A
   E853 C9            [10]  621     RET
                            622 ;
   E854 ED 4B 37 E8   [20]  623 FLASHE: LD  BC,(FLASHA)
   E858 3A 3A E8      [13]  624     LD  A,(FLASHD)
   E85B ED 79         [12]  625     OUT (C),A
   E85D C9            [10]  626     RET
                            627 ;
                            628 ;
   E85E C5            [11]  629 FLSHGT: PUSH    BC
   E85F D5            [11]  630     PUSH    DE
   E860 E5            [11]  631     PUSH    HL
   E861 CD 1A E8      [17]  632     CALL    FLASHS
   E864 CD 36 E8      [17]  633 FLGTLP: CALL    FLASHG
   E867 CD 9C E8      [17]  634     CALL    KEYSEN
   E86A 28 1B         [12]  635     JR  Z,FLSHL3
   E86C CD DD E8      [17]  636     CALL    NEWCEK
   E86F 3E 00         [ 7]  637     LD  A,#0
   E871 20 20         [12]  638     JR  NZ,FLSHRT
   E873 CD F3 E8      [17]  639     CALL    SAMECK
   E876 20 EC         [12]  640     JR  NZ,FLGTLP
   E878 21                  641     .db 0x21        ; LD HL,
   E879 00 00               642 REPETF: .dw 0       ;
   E87B 2B            [ 6]  643     DEC HL      ;+
   E87C 22 79 E8      [16]  644     LD  (#REPETF),HL
   E87F 7C            [ 4]  645     LD  A,H
   E880 B5            [ 4]  646     OR  L
   E881 20 E1         [12]  647     JR  NZ,FLGTLP
   E883 3E 01         [ 7]  648     LD  A,#1
   E885 18 0C         [12]  649     JR  FLSHRT
   E887 CD 03 E9      [17]  650 FLSHL3: CALL    NEWFLG
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 13.
Hexadecimal [16-Bits]



   E88A CD 36 E8      [17]  651 FLGTL2: CALL    FLASHG
   E88D CD 9C E8      [17]  652     CALL    KEYSEN
   E890 28 F8         [12]  653     JR  Z,FLGTL2
   E892 AF            [ 4]  654     XOR A
   E893 F5            [11]  655 FLSHRT: PUSH    AF
   E894 CD 54 E8      [17]  656     CALL    FLASHE
   E897 F1            [10]  657     POP AF
   E898 E1            [10]  658     POP HL
   E899 D1            [10]  659     POP DE
   E89A C1            [10]  660     POP BC
   E89B C9            [10]  661     RET
                            662     
                            663 ;
                            664 ;
                            665 ;
   E89C CD AE E8      [17]  666 KEYSEN: CALL    KEYSN2
   E89F 06 04         [ 7]  667 KEYSN0: LD  B,#4
   E8A1 CD AE E8      [17]  668 KEYSN1: CALL    KEYSN2
   E8A4 CB 4B         [ 8]  669     BIT 1,E
   E8A6 20 F7         [12]  670     JR  NZ,KEYSN0
   E8A8 10 F7         [13]  671     DJNZ    KEYSN1
   E8AA 7B            [ 4]  672     LD  A,E
   E8AB E6 01         [ 7]  673     AND #1
   E8AD C9            [10]  674     RET
                            675 ;
   E8AE 21 58 EE      [10]  676 KEYSN2: LD  HL,#NEWBUF
   E8B1 C5            [11]  677     PUSH    BC
   E8B2 01 09 80      [10]  678     LD  BC,#0x8009  ;KEYIO
   E8B5 11 00 09      [10]  679     LD  DE,#0x900
   E8B8 ED 78         [12]  680 KEYSN3: IN  A,(C)
   E8BA AE            [ 7]  681     XOR (HL)
   E8BB 28 02         [12]  682     JR  Z,L0D9D
   E8BD CB CB         [ 8]  683     SET 1,E
   E8BF AE            [ 7]  684 L0D9D:  XOR (HL)
   E8C0 77            [ 7]  685     LD  (HL),A
   E8C1 2F            [ 4]  686     CPL
   E8C2 B7            [ 4]  687     OR  A
   E8C3 28 02         [12]  688     JR  Z,L0DA5
   E8C5 CB C3         [ 8]  689     SET 0,E
   E8C7 23            [ 6]  690 L0DA5:  INC HL
   E8C8 0B            [ 6]  691     DEC BC
   E8C9 15            [ 4]  692     DEC D
   E8CA 20 EC         [12]  693     JR  NZ,KEYSN3
   E8CC ED 78         [12]  694     IN  A,(C)
   E8CE AE            [ 7]  695     XOR (HL)
   E8CF 28 02         [12]  696     JR  Z,L0DB1
   E8D1 CB CB         [ 8]  697     SET 1,E
   E8D3 AE            [ 7]  698 L0DB1:  XOR (HL)
   E8D4 77            [ 7]  699     LD  (HL),A
   E8D5 E6 12         [ 7]  700     AND #0x12
   E8D7 20 02         [12]  701     JR  NZ,L0DB9
   E8D9 CB C3         [ 8]  702     SET 0,E
   E8DB C1            [10]  703 L0DB9:  POP BC
   E8DC C9            [10]  704     RET
                            705 ;
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 14.
Hexadecimal [16-Bits]



   E8DD 21 4E EE      [10]  706 NEWCEK: LD  HL,#OLDBUF
   E8E0 11 58 EE      [10]  707     LD  DE,#NEWBUF
   E8E3 06 09         [ 7]  708     LD  B,#9
   E8E5 1A            [ 7]  709 NEWPL1: LD  A,(DE)
   E8E6 2F            [ 4]  710     CPL
   E8E7 A6            [ 7]  711     AND (HL)
   E8E8 C0            [11]  712     RET NZ
   E8E9 13            [ 6]  713     INC DE
   E8EA 23            [ 6]  714     INC HL
   E8EB 10 F8         [13]  715     DJNZ    NEWPL1
   E8ED 1A            [ 7]  716     LD  A,(DE)
   E8EE 2F            [ 4]  717     CPL
   E8EF A6            [ 7]  718     AND (HL)
   E8F0 E6 10         [ 7]  719     AND #0x10
   E8F2 C9            [10]  720     RET
                            721 ;
   E8F3 21 4E EE      [10]  722 SAMECK: LD  HL,#OLDBUF
   E8F6 11 58 EE      [10]  723     LD  DE,#NEWBUF
   E8F9 06 0A         [ 7]  724     LD  B,#10
   E8FB 1A            [ 7]  725 SAMELP: LD  A,(DE)
   E8FC AE            [ 7]  726     XOR (HL)
   E8FD C0            [11]  727     RET NZ
   E8FE 13            [ 6]  728     INC DE
   E8FF 23            [ 6]  729     INC HL
   E900 10 F9         [13]  730     DJNZ    SAMELP
   E902 C9            [10]  731     RET
                            732 ;
   E903 21 58 EE      [10]  733 NEWFLG: LD  HL,#NEWBUF
   E906 11 4E EE      [10]  734     LD  DE,#OLDBUF
   E909 06 09         [ 7]  735     LD  B,#9
   E90B 1A            [ 7]  736 FLGLP1: LD  A,(DE)
   E90C 2F            [ 4]  737     CPL
   E90D B6            [ 7]  738     OR  (HL)
   E90E 4E            [ 7]  739     LD  C,(HL)
   E90F 77            [ 7]  740     LD  (HL),A
   E910 79            [ 4]  741     LD  A,C
   E911 12            [ 7]  742     LD  (DE),A
   E912 13            [ 6]  743     INC DE
   E913 23            [ 6]  744     INC HL
   E914 10 F5         [13]  745     DJNZ    FLGLP1
   E916 7E            [ 7]  746     LD  A,(HL)
   E917 12            [ 7]  747     LD  (DE),A
   E918 11 58 EE      [10]  748     LD  DE,#NEWBUF
   E91B 21 4E EE      [10]  749     LD  HL,#OLDBUF
   E91E 01 00 09      [10]  750     LD  BC,#0x0900
   E921 1A            [ 7]  751 FLGLP2: LD  A,(DE)
   E922 2F            [ 4]  752     CPL
   E923 B7            [ 4]  753     OR  A
   E924 28 0A         [12]  754     JR  Z,FLGLP3
   E926 0D            [ 4]  755     DEC C
   E927 20 0C         [12]  756     JR  NZ,FLGLP5
   E929 B6            [ 7]  757     OR  (HL)
   E92A 77            [ 7]  758     LD  (HL),A
   E92B 3E FF         [ 7]  759     LD  A,#0xFF
   E92D 12            [ 7]  760     LD  (DE),A
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 15.
Hexadecimal [16-Bits]



   E92E 0E 01         [ 7]  761 FLGLP4: LD  C,#1
   E930 13            [ 6]  762 FLGLP3: INC DE
   E931 23            [ 6]  763     INC HL
   E932 10 ED         [13]  764     DJNZ    FLGLP2
   E934 C9            [10]  765     RET
   E935 C5            [11]  766 FLGLP5: PUSH    BC
   E936 01 01 08      [10]  767     LD  BC,#0x801
   E939 0F            [ 4]  768 FLGLP6: RRCA
   E93A 38 0B         [12]  769     JR  C,FLGLP7
   E93C CB 01         [ 8]  770     RLC C
   E93E 10 F9         [13]  771     DJNZ    FLGLP6
   E940 C1            [10]  772     POP BC
   E941 18 EB         [12]  773     JR  FLGLP4
   E943 0F            [ 4]  774 FLGLP9: RRCA
   E944 DC 4E E9      [17]  775     CALL    C,FLGLP8
   E947 CB 01         [ 8]  776 FLGLP7: RLC C
   E949 10 F8         [13]  777     DJNZ    FLGLP9
   E94B C1            [10]  778     POP BC
   E94C 18 E0         [12]  779     JR  FLGLP4
                            780 ;
   E94E F5            [11]  781 FLGLP8: PUSH    AF
   E94F 79            [ 4]  782     LD  A,C
   E950 B6            [ 7]  783     OR  (HL)
   E951 77            [ 7]  784     LD  (HL),A
   E952 1A            [ 7]  785     LD  A,(DE)
   E953 B1            [ 4]  786     OR  C
   E954 12            [ 7]  787     LD  (DE),A
   E955 F1            [10]  788     POP AF
   E956 C9            [10]  789     RET
                            790 ;
                            791 ;
   E957 7E            [ 7]  792 KYBFST: LD  A,(HL)
   E958 22 DD E7      [16]  793     LD  (KEYBFA),HL
   E95B 32 E0 E7      [13]  794     LD  (KEYBFD),A
   E95E 4A            [ 4]  795     LD  C,D
   E95F 06 00         [ 7]  796     LD  B,#0
   E961 21 0B F0      [10]  797     LD  HL,#KEYDT1  ;NORMAL DATA
   E964 3A 61 EE      [13]  798     LD  A,(NEWMOD)
   E967 CB 4F         [ 8]  799     BIT 1,A
   E969 20 03         [12]  800     JR  NZ,L0E4C
   E96B 21 53 F0      [10]  801     LD  HL,#KEYDT2  ;SHIFT DATA
   E96E 09            [11]  802 L0E4C:  ADD HL,BC
   E96F 4E            [ 7]  803     LD  C,(HL)
   E970 47            [ 4]  804     LD  B,A
   E971 CB 50         [ 8]  805     BIT 2,B
   E973 20 09         [12]  806     JR  NZ,CTRLNO
   E975 79            [ 4]  807     LD  A,C
   E976 D6 40         [ 7]  808     SUB #0x40
   E978 38 17         [12]  809     JR  C,NONKEZ
   E97A E6 1F         [ 7]  810     AND #0x1F
   E97C 18 0D         [12]  811     JR  SETKY
   E97E CB 70         [ 8]  812 CTRLNO: BIT 6,B
   E980 20 1A         [12]  813     JR  NZ,SETKY2
   E982 79            [ 4]  814     LD  A,C
   E983 D6 40         [ 7]  815     SUB #0x40
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 16.
Hexadecimal [16-Bits]



   E985 38 07         [12]  816     JR  C,NONKEY
   E987 E6 1F         [ 7]  817     AND #0x1F
   E989 C6 80         [ 7]  818     ADD A,#0x80
   E98B CD D3 E9      [17]  819 SETKY:  CALL    BFSTSB
   E98E C3 0F E8      [10]  820 NONKEY: JP  KYBFSR
   E991 C6 10         [ 7]  821 NONKEZ: ADD A,#0x10
   E993 FE 04         [ 7]  822     CP  #4
   E995 30 F7         [12]  823     JR  NC,NONKEY
   E997 32 07 EF      [13]  824     LD  (COLORF),A
   E99A 18 F2         [12]  825     JR  NONKEY
                            826 ;
                            827 ;
   E99C 79            [ 4]  828 SETKY2: LD  A,C
   E99D FE F0         [ 7]  829     CP  #0xF0
   E99F 30 16         [12]  830     JR  NC,FUNCTN
   E9A1 3A 63 EE      [13]  831     LD  A,(#LOCKMD)
   E9A4 B7            [ 4]  832     OR  A
   E9A5 79            [ 4]  833     LD  A,C
   E9A6 28 E3         [12]  834     JR  Z,SETKY
   E9A8 FE 41         [ 7]  835     CP  #0x41
   E9AA 38 DF         [12]  836     JR  C,SETKY
   E9AC E6 1F         [ 7]  837     AND #0x1F
   E9AE FE 1B         [ 7]  838     CP  #0x1B
   E9B0 79            [ 4]  839     LD  A,C
   E9B1 30 D8         [12]  840     JR  NC,SETKY
   E9B3 EE 20         [ 7]  841     XOR #0x20
   E9B5 18 D4         [12]  842     JR  SETKY
   E9B7 D6 F1         [ 7]  843 FUNCTN: SUB #0xF1
   E9B9 6F            [ 4]  844     LD  L,A
   E9BA 26 00         [ 7]  845     LD  H,#0
   E9BC 29            [11]  846     ADD HL,HL
   E9BD 29            [11]  847     ADD HL,HL
   E9BE 29            [11]  848     ADD HL,HL
   E9BF 29            [11]  849     ADD HL,HL
   E9C0 01 6B EF      [10]  850     LD  BC,#FUNBUF
   E9C3 09            [11]  851     ADD HL,BC
   E9C4 46            [ 7]  852     LD  B,(HL)
   E9C5 23            [ 6]  853     INC HL
   E9C6 78            [ 4]  854     LD  A,B
   E9C7 B7            [ 4]  855     OR  A
   E9C8 28 C4         [12]  856     JR  Z,NONKEY
   E9CA 7E            [ 7]  857 FUNCLP: LD  A,(HL)
   E9CB CD D3 E9      [17]  858     CALL    BFSTSB
   E9CE 23            [ 6]  859     INC HL
   E9CF 10 F9         [13]  860     DJNZ    FUNCLP
   E9D1 18 BB         [12]  861     JR  NONKEY
   E9D3 E5            [11]  862 BFSTSB: PUSH    HL
   E9D4 C5            [11]  863     PUSH    BC
   E9D5 4F            [ 4]  864     LD  C,A
   E9D6 2A 64 EE      [16]  865     LD  HL,(POINT1)
   E9D9 7C            [ 4]  866     LD  A,H
   E9DA 3C            [ 4]  867     INC A
   E9DB E6 3F         [ 7]  868     AND #0x3F
   E9DD BD            [ 4]  869     CP  L
   E9DE 28 0C         [12]  870     JR  Z,NONSET
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 17.
Hexadecimal [16-Bits]



   E9E0 32 65 EE      [13]  871     LD  (POINT2),A
   E9E3 6F            [ 4]  872     LD  L,A
   E9E4 26 00         [ 7]  873     LD  H,#0
   E9E6 79            [ 4]  874     LD  A,C
   E9E7 01 66 EE      [10]  875     LD  BC,#INKYBF
   E9EA 09            [11]  876     ADD HL,BC
   E9EB 77            [ 7]  877     LD  (HL),A
   E9EC C1            [10]  878 NONSET: POP BC
   E9ED E1            [10]  879     POP HL
   E9EE C9            [10]  880 NORET:  RET 
                            881 ; VIDEO RAM ADDRESS CALCULATION
   E9EF 2A 4C EE      [16]  882 ADRCAL: LD  HL,(CURX)
   E9F2 AF            [ 4]  883 ADRCA2: XOR A
   E9F3 CB 3C         [ 8]  884     SRL H
   E9F5 1F            [ 4]  885     RRA
   E9F6 CB 3C         [ 8]  886     SRL H
   E9F8 1F            [ 4]  887     RRA
   E9F9 CB 3C         [ 8]  888     SRL H
   E9FB 1F            [ 4]  889     RRA
   E9FC B5            [ 4]  890     OR  L
   E9FD 01 00 00      [10]  891 PAGE05: LD  BC,#0       ;OFSET
   EA00 81            [ 4]  892     ADD A,C
   EA01 4F            [ 4]  893     LD  C,A
   EA02 78            [ 4]  894     LD  A,B
   EA03 8C            [ 4]  895     ADC A,H
   EA04 47            [ 4]  896     LD  B,A
   EA05 C9            [10]  897     RET
   EA06                     898 ACCPRT1:
   EA06 F5            [11]  899     PUSH    AF
   EA07 C5            [11]  900     PUSH    BC
   EA08 D5            [11]  901     PUSH    DE
   EA09 E5            [11]  902     PUSH    HL
   EA0A CD 12 EA      [17]  903     CALL    ACCOUS
   EA0D E1            [10]  904     POP HL
   EA0E D1            [10]  905     POP DE
   EA0F C1            [10]  906     POP BC
   EA10 F1            [10]  907     POP AF
   EA11                     908 CTRLSB: 
   EA11 C9            [10]  909     RET
                            910 ;
   EA12                     911 ACCOUS: 
   EA12                     912 ACCOUT: 
   EA12 FE 20         [ 7]  913     CP  #' '        ;020h
   EA14 DA 11 EA      [10]  914     JP  C,CTRLSB
   EA17 F5            [11]  915 ACCDIS: PUSH    AF
   EA18 3A 06 EF      [13]  916     LD  A,(GMODEF)  ;GMODEF 2,3 X
   EA1B FE 02         [ 7]  917     CP  #2
   EA1D 38 06         [12]  918     JR  C,ACCOL1
   EA1F FE 04         [ 7]  919     CP  #4
   EA21 C1            [10]  920     POP BC
   EA22 D8            [11]  921     RET C
                            922 ;   CALL    GACPRT
   EA23 18 07         [12]  923     JR  ACCOL2
   EA25 CD EF E9      [17]  924 ACCOL1: CALL    ADRCAL
   EA28 F1            [10]  925     POP AF
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 18.
Hexadecimal [16-Bits]



   EA29 CD 1C EB      [17]  926     CALL    BCOUTA 
   EA2C 2A 4C EE      [16]  927 ACCOL2: LD  HL,(CURX)
   EA2F 2C            [ 4]  928     INC L
   EA30 7D            [ 4]  929     LD  A,L
   EA31 FE 20         [ 7]  930     CP  #0x20       ;다음줄 체크
   EA33 38 35         [12]  931     JR  C,#CURSET
   EA35 24            [ 4]  932     INC H
   EA36 06 01         [ 7]  933     LD  B,#1
   EA38 7C            [ 4]  934 CTRLM0: LD  A,H
   EA39 FE 10         [ 7]  935     CP  #16
   EA3B 38 24         [12]  936     JR  C,CURSET0
   EA3D C5            [11]  937     PUSH    BC      ;16줄(1페이지) 체크
   EA3E 01 00 02      [10]  938 PAGE02: LD  BC,#0x0200  ;CLR CHR
   EA41 3E 00         [ 7]  939     LD  A,#0            ; 소스는 020h
   EA43 16 20         [ 7]  940     LD  D,#0x20
   EA45 ED 79         [12]  941 SCRLC1: OUT (C),A
   EA47 03            [ 6]  942     INC BC
   EA48 15            [ 4]  943     DEC D
   EA49 20 FA         [12]  944     JR  NZ,SCRLC1
   EA4B 01 00 0A      [10]  945 PAGE04: LD  BC,#0x0A00
   EA4E 3A 07 EF      [13]  946     LD  A,(COLORF)
   EA51 E6 03         [ 7]  947     AND #0x03
   EA53 16 20         [ 7]  948     LD  D,#0x20
   EA55 ED 79         [12]  949 SCRLC2: OUT (C),A
   EA57 03            [ 6]  950     INC BC
   EA58 15            [ 4]  951     DEC D
   EA59 20 FA         [12]  952     JR  NZ,SCRLC2
   EA5B CD 6E EA      [17]  953     CALL    SCRLUP
   EA5E 26 0F         [ 7]  954     LD  H,#15
   EA60 C1            [10]  955     POP BC
   EA61 7C            [ 4]  956 CURSET0: LD A,H
   EA62 E5            [11]  957     PUSH    HL
   EA63 CD 6C ED      [17]  958     CALL    ATBHL
   EA66 70            [ 7]  959     LD  (HL),B
   EA67 E1            [10]  960     POP HL
   EA68 2E 00         [ 7]  961     LD  L,#0
   EA6A 22 4C EE      [16]  962 CURSET: LD  (CURX),HL
   EA6D C9            [10]  963     RET
                            964 ;
                            965 ;
   EA6E D9            [ 4]  966 SCRLUP: EXX
   EA6F C5            [11]  967     PUSH    BC
   EA70 D5            [11]  968     PUSH    DE
   EA71 E5            [11]  969     PUSH    HL
   EA72 ED 4B A6 EB   [20]  970     LD  BC,(CHRSTA)
   EA76 ED 5B D9 EA   [20]  971     LD  DE,(CHRLE2)
   EA7A CD 9B EA      [17]  972     CALL    BCLDIR
   EA7D ED 4B B6 EB   [20]  973     LD  BC,(ATRSTA)
   EA81 ED 5B D9 EA   [20]  974     LD  DE,(CHRLE2)
   EA85 CD 9B EA      [17]  975     CALL    BCLDIR
                            976 ;LINE FLG SCROLL
   EA88 ED 5B C9 EB   [20]  977     LD  DE,(LINEFR)
   EA8C 62            [ 4]  978     LD  H,D
   EA8D 6B            [ 4]  979     LD  L,E
   EA8E 23            [ 6]  980     INC HL
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 19.
Hexadecimal [16-Bits]



   EA8F 01 3F 00      [10]  981 PAGE11: LD  BC,#63
   EA92 1A            [ 7]  982     LD  A,(DE)
   EA93 ED B0         [21]  983     LDIR
   EA95 12            [ 7]  984     LD  (DE),A
   EA96 E1            [10]  985     POP HL
   EA97 D1            [10]  986     POP DE
   EA98 C1            [10]  987     POP BC
   EA99 D9            [ 4]  988     EXX
   EA9A C9            [10]  989     RET
                            990 ;
   EA9B C5            [11]  991 BCLDIR: PUSH    BC
   EA9C D9            [ 4]  992     EXX
   EA9D C1            [10]  993     POP BC
   EA9E 21 09 EF      [10]  994     LD  HL,#SCRLBF
   EAA1 1E 20         [ 7]  995     LD  E,#0x20
   EAA3 ED 78         [12]  996 BCLDL1: IN  A,(C)
   EAA5 77            [ 7]  997     LD  (HL),A
   EAA6 03            [ 6]  998     INC BC
   EAA7 23            [ 6]  999     INC HL
   EAA8 1D            [ 4] 1000     DEC E
   EAA9 20 F8         [12] 1001     JR  NZ,BCLDL1
   EAAB ED 78         [12] 1002 BCLDL2: IN  A,(C)
   EAAD 03            [ 6] 1003     INC BC
   EAAE D9            [ 4] 1004     EXX
   EAAF ED 79         [12] 1005     OUT (C),A
   EAB1 03            [ 6] 1006     INC BC
   EAB2 1B            [ 6] 1007     DEC DE
   EAB3 7A            [ 4] 1008     LD  A,D
   EAB4 B3            [ 4] 1009     OR  E
   EAB5 D9            [ 4] 1010     EXX
   EAB6 20 F3         [12] 1011     JR  NZ,BCLDL2
   EAB8 D9            [ 4] 1012     EXX
   EAB9 21 09 EF      [10] 1013     LD  HL,#SCRLBF
   EABC 1E 20         [ 7] 1014     LD  E,#0x20
   EABE 7E            [ 7] 1015 BCLDL3: LD  A,(HL)
   EABF ED 79         [12] 1016     OUT (C),A
   EAC1 23            [ 6] 1017     INC HL
   EAC2 03            [ 6] 1018     INC BC
   EAC3 1D            [ 4] 1019     DEC E
   EAC4 20 F8         [12] 1020     JR  NZ,BCLDL3
   EAC6 C9            [10] 1021     RET
                           1022 ;
                           1023 ;
                           1024 ;SCROLL DOWN
   EAC7 D9            [ 4] 1025 SCRLDW: EXX
   EAC8 C5            [11] 1026     PUSH    BC
   EAC9 D5            [11] 1027     PUSH    DE
   EACA E5            [11] 1028     PUSH    HL
   EACB 01 FF 07      [10] 1029 PAGE12: LD  BC,#0x07FF  ;★원래는 DEPRTR
   EACE ED 5B D9 EA   [20] 1030     LD  DE,(CHRLE2)
   EAD2 CD F0 EA      [17] 1031     CALL    BCLDDR
   EAD5 01 FF 0F      [10] 1032 PAGE14: LD  BC,#0x0FFF
   EAD8 11                 1033     .db     0x11        ; LD DE,
   EAD9 E0 07              1034 CHRLE2: .dw 0x07E0
   EADB CD F0 EA      [17] 1035     CALL    BCLDDR
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 20.
Hexadecimal [16-Bits]



                           1036 ;LINE FLG SCROLL
   EADE 11 68 EF      [10] 1037 PAGE16: LD  DE,#LINEF+63
   EAE1 6B            [ 4] 1038     LD  L,E
   EAE2 62            [ 4] 1039     LD  H,D
   EAE3 2B            [ 6] 1040     DEC HL
   EAE4 01 3F 00      [10] 1041 PAGE17: LD  BC,#63
   EAE7 1A            [ 7] 1042     LD  A,(DE)
   EAE8 ED B8         [21] 1043     LDDR
   EAEA 12            [ 7] 1044     LD  (DE),A
   EAEB E1            [10] 1045     POP HL
   EAEC D1            [10] 1046     POP DE
   EAED C1            [10] 1047     POP BC
   EAEE D9            [ 4] 1048     EXX
   EAEF C9            [10] 1049     RET
                           1050 ;
   EAF0 C5            [11] 1051 BCLDDR: PUSH    BC
   EAF1 D9            [ 4] 1052     EXX
   EAF2 C1            [10] 1053     POP BC
   EAF3 21 28 EF      [10] 1054     LD  HL,#SCRLBF+0x1F
   EAF6 1E 20         [ 7] 1055     LD  E,#0x20
   EAF8 ED 78         [12] 1056 BCLDD1: IN  A,(C)
   EAFA 77            [ 7] 1057     LD  (HL),A
   EAFB 0B            [ 6] 1058     DEC BC
   EAFC 2B            [ 6] 1059     DEC HL
   EAFD 1D            [ 4] 1060     DEC E
   EAFE 20 F8         [12] 1061     JR  NZ,BCLDD1
   EB00 ED 78         [12] 1062 BCLDD2: IN  A,(C)
   EB02 0B            [ 6] 1063     DEC BC
   EB03 D9            [ 4] 1064     EXX
   EB04 ED 79         [12] 1065     OUT (C),A
   EB06 0B            [ 6] 1066     DEC BC
   EB07 1B            [ 6] 1067     DEC DE
   EB08 7A            [ 4] 1068     LD  A,D
   EB09 B3            [ 4] 1069     OR  E
   EB0A D9            [ 4] 1070     EXX
   EB0B 20 F3         [12] 1071     JR  NZ,BCLDD2
   EB0D D9            [ 4] 1072     EXX
   EB0E 21 28 EF      [10] 1073     LD  HL,#SCRLBF+0x1F
   EB11 1E 20         [ 7] 1074     LD  E,#0x20
   EB13 7E            [ 7] 1075 BCLDD3: LD  A,(HL)
   EB14 ED 79         [12] 1076     OUT (C),A
   EB16 2B            [ 6] 1077     DEC HL
   EB17 0B            [ 6] 1078     DEC BC
   EB18 1D            [ 4] 1079     DEC E
   EB19 20 F8         [12] 1080     JR  NZ,BCLDD3
   EB1B C9            [10] 1081     RET 
                           1082 ;
                           1083 ; 화면에 문자 출력
   EB1C FE 60         [ 7] 1084 BCOUTA: CP  #'a'-1      ;060h
   EB1E 38 14         [12] 1085     JR  C,BCOTOK
   EB20 FE E0         [ 7] 1086     CP  #0xE0
   EB22 30 1C         [12] 1087     JR  NC,SEMIOT
   EB24 E6 7F         [ 7] 1088     AND #0x7F
   EB26 ED 79         [12] 1089     OUT (C),A
   EB28 CB D8         [ 8] 1090     SET 3,B
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 21.
Hexadecimal [16-Bits]



   EB2A 3A 07 EF      [13] 1091     LD  A,(COLORF)
   EB2D E6 03         [ 7] 1092     AND #3
   EB2F F6 08         [ 7] 1093     OR  #8
   EB31 ED 79         [12] 1094     OUT (C),A
   EB33 C9            [10] 1095     RET
                           1096 ;
   EB34 ED 79         [12] 1097 BCOTOK: OUT (C),A
   EB36 CB D8         [ 8] 1098     SET 3,B
   EB38 3A 07 EF      [13] 1099     LD  A,(COLORF)
   EB3B E6 03         [ 7] 1100     AND #3
   EB3D ED 79         [12] 1101     OUT (C),A
   EB3F C9            [10] 1102     RET
                           1103 ;
   EB40 E6 0F         [ 7] 1104 SEMIOT: AND #0x0F
   EB42 D5            [11] 1105     PUSH    DE
   EB43 57            [ 4] 1106     LD  D,A
   EB44 3A 07 EF      [13] 1107     LD  A,(COLORF)
   EB47 3D            [ 4] 1108     DEC A
   EB48 E6 07         [ 7] 1109     AND #7
   EB4A 07            [ 4] 1110     RLCA
   EB4B 07            [ 4] 1111     RLCA
   EB4C 07            [ 4] 1112     RLCA
   EB4D 07            [ 4] 1113     RLCA
   EB4E B2            [ 4] 1114     OR  D
   EB4F D1            [10] 1115     POP DE
   EB50 ED 79         [12] 1116     OUT (C),A
   EB52 CB D8         [ 8] 1117     SET 3,B
   EB54 3A 07 EF      [13] 1118     LD  A,(COLORF)
   EB57 E6 03         [ 7] 1119     AND #3
   EB59 F6 04         [ 7] 1120     OR  #4
   EB5B ED 79         [12] 1121     OUT (C),A
   EB5D C9            [10] 1122     RET 
                           1123 
                           1124 ;
   EB5E CD EF E9      [17] 1125 DELETE: CALL    ADRCAL
   EB61 C5            [11] 1126     PUSH    BC
   EB62 CD 5B ED      [17] 1127     CALL    ECUYST
   EB65 63            [ 4] 1128     LD  H,E
   EB66 2E 00         [ 7] 1129     LD  L,#0
   EB68 CD F2 E9      [17] 1130     CALL    ADRCA2
   EB6B D1            [10] 1131     POP DE
   EB6C 69            [ 4] 1132     LD  L,C
   EB6D 60            [ 4] 1133     LD  H,B
   EB6E B7            [ 4] 1134     OR  A
   EB6F ED 52         [15] 1135     SBC HL,DE
   EB71 16 00         [ 7] 1136     LD  D,#0    ;★구버전,신버전 00H,20H ???
   EB73 3A 07 EF      [13] 1137     LD  A,(COLORF)
   EB76 E6 03         [ 7] 1138     AND #3
   EB78 5F            [ 4] 1139     LD  E,A
   EB79 0B            [ 6] 1140 DELLP1: DEC BC
   EB7A ED 78         [12] 1141     IN  A,(C)
   EB7C ED 51         [12] 1142     OUT (C),D
   EB7E 57            [ 4] 1143     LD  D,A
   EB7F CB D8         [ 8] 1144     SET 3,B
   EB81 ED 78         [12] 1145     IN  A,(C)
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 22.
Hexadecimal [16-Bits]



   EB83 ED 59         [12] 1146     OUT (C),E
   EB85 5F            [ 4] 1147     LD  E,A
   EB86 CB 98         [ 8] 1148     RES 3,B
   EB88 2B            [ 6] 1149     DEC HL
   EB89 7C            [ 4] 1150     LD  A,H
   EB8A B5            [ 4] 1151     OR  L
   EB8B 20 EC         [12] 1152     JR  NZ,DELLP1
   EB8D 3A 4C EE      [13] 1153     LD  A,(CURX)
   EB90 B7            [ 4] 1154     OR  A
   EB91 20 08         [12] 1155     JR  NZ,DELLP2
   EB93 D5            [11] 1156     PUSH    DE
   EB94 CD 69 ED      [17] 1157     CALL    CUTBHL
   EB97 7E            [ 7] 1158     LD  A,(HL)
   EB98 D1            [10] 1159     POP DE
   EB99 B7            [ 4] 1160     OR  A
   EB9A C8            [11] 1161     RET Z
   EB9B 0B            [ 6] 1162 DELLP2: DEC BC
   EB9C ED 51         [12] 1163     OUT (C),D
   EB9E CB D8         [ 8] 1164     SET 3,B
   EBA0 ED 59         [12] 1165     OUT (C),E
   EBA2 C3 17 EC      [10] 1166     JP  LEFT
                           1167 
   EBA5                    1168 CLR2:   
   EBA5 01                 1169     .db 1       ;LD BC,(CHRSTA)
   EBA6 00 00              1170 CHRSTA: .dw 0x0     ;SEMIGRAPHIC ADR.+
   EBA8 2A B9 EB      [16] 1171     LD  HL,(CHRLEN)
   EBAB 16 00         [ 7] 1172     LD  D,#0        ;★구버전,신버전 00H,20H ???
   EBAD ED 51         [12] 1173 CLRLP:  OUT (C),D
   EBAF 03            [ 6] 1174     INC BC
   EBB0 2B            [ 6] 1175     DEC HL
   EBB1 7C            [ 4] 1176     LD  A,H
   EBB2 B5            [ 4] 1177     OR  L
   EBB3 20 F8         [12] 1178     JR  NZ,CLRLP
   EBB5 01                 1179     .db 1       ;LD BC,(ATRSTA)
   EBB6 00 08              1180 ATRSTA: .dw 0x0800
   EBB8 21                 1181     .db 0x21        ;LD HL,(CHRLEN)
   EBB9 00                 1182 CHRLEN: .db 0x0800
   EBBA 3A 07 EF      [13] 1183     LD  A,(COLORF)
   EBBD E6 03         [ 7] 1184     AND #3
   EBBF 57            [ 4] 1185     LD  D,A
   EBC0 ED 51         [12] 1186 ATRCLS: OUT (C),D
   EBC2 03            [ 6] 1187     INC BC
   EBC3 2B            [ 6] 1188     DEC HL
   EBC4 7C            [ 4] 1189     LD  A,H
   EBC5 B5            [ 4] 1190     OR  L
   EBC6 20 F8         [12] 1191     JR  NZ,ATRCLS
   EBC8 21                 1192     .db 0x21        ;LD HL,(LINEFR)
   EBC9 29 EF              1193 LINEFR: .dw LINEF
   EBCB 06 41         [ 7] 1194 PAGE23: LD  B,#65
   EBCD 36 00         [10] 1195 LINCLS: LD  (HL),#0
   EBCF 23            [ 6] 1196     INC HL
   EBD0 10 FB         [13] 1197     DJNZ    LINCLS
   EBD2 21 00 00      [10] 1198 SCRHOME:    LD  HL,#0
   EBD5 C3 52 EC      [10] 1199     JP  CURST2
                           1200 ;
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 23.
Hexadecimal [16-Bits]



   EBD8 CD 5B ED      [17] 1201 INST:   CALL    ECUYST
   EBDB 63            [ 4] 1202     LD  H,E
   EBDC 2E 00         [ 7] 1203     LD  L,#0
   EBDE CD F2 E9      [17] 1204     CALL    ADRCA2
   EBE1 0B            [ 6] 1205     DEC BC
   EBE2 CD 2A ED      [17] 1206     CALL    AINBC
   EBE5 FE 21         [ 7] 1207     CP  #0x21
   EBE7 D0            [11] 1208     RET NC
   EBE8 C5            [11] 1209     PUSH    BC
   EBE9 CD EF E9      [17] 1210     CALL    ADRCAL
   EBEC E1            [10] 1211     POP HL
   EBED B7            [ 4] 1212     OR  A
   EBEE ED 42         [15] 1213     SBC HL,BC
   EBF0 16 00         [ 7] 1214     LD  D,#0x00     ;★구버전,신버전 00H,20H ???
   EBF2 3A 07 EF      [13] 1215     LD  A,(COLORF)
   EBF5 E6 03         [ 7] 1216     AND #3
   EBF7 5F            [ 4] 1217     LD  E,A
   EBF8 23            [ 6] 1218     INC HL
   EBF9 ED 78         [12] 1219 INSTL1: IN  A,(C)
   EBFB ED 51         [12] 1220     OUT (C),D
   EBFD 57            [ 4] 1221     LD  D,A
   EBFE CB D8         [ 8] 1222     SET 3,B
   EC00 ED 78         [12] 1223     IN  A,(C)
   EC02 ED 59         [12] 1224     OUT (C),E
   EC04 5F            [ 4] 1225     LD  E,A
   EC05 CB 98         [ 8] 1226     RES 3,B
   EC07 03            [ 6] 1227     INC BC
   EC08 2B            [ 6] 1228     DEC HL
   EC09 7C            [ 4] 1229     LD  A,H
   EC0A B5            [ 4] 1230     OR  L
   EC0B 20 EC         [12] 1231     JR  NZ,INSTL1
   EC0D C9            [10] 1232     RET
                           1233 ;
   EC0E 3E 01         [ 7] 1234 LOCK:   LD  A,#1
   EC10 32 63 EE      [13] 1235 LOCKST: LD  (LOCKMD),A
   EC13 C9            [10] 1236     RET
                           1237 ;
   EC14 AF            [ 4] 1238 UNLOCK: XOR A
   EC15 18 F9         [12] 1239     JR  LOCKST
                           1240 ;
   EC17 2A 4C EE      [16] 1241 LEFT:   LD  HL,(CURX)
   EC1A 2D            [ 4] 1242     DEC L
   EC1B 7D            [ 4] 1243     LD  A,L
   EC1C FE FF         [ 7] 1244     CP  #0xFF
   EC1E 20 32         [12] 1245     JR  NZ,CURST2
   EC20 2E 1F         [ 7] 1246     LD  L,#0x1F
   EC22 25            [ 4] 1247 UPCUR:  DEC H
   EC23 7C            [ 4] 1248     LD  A,H
   EC24 FE FF         [ 7] 1249     CP  #0xFF
   EC26 20 2A         [12] 1250     JR  NZ,CURST2
   EC28 E5            [11] 1251     PUSH    HL
   EC29 CD C7 EA      [17] 1252     CALL    SCRLDW
   EC2C E1            [10] 1253     POP HL
   EC2D 26 00         [ 7] 1254     LD  H,#0
   EC2F 18 21         [12] 1255     JR  CURST2
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 24.
Hexadecimal [16-Bits]



                           1256 ;
   EC31 CD 5B ED      [17] 1257 CTRLM:  CALL    ECUYST
   EC34 63            [ 4] 1258     LD  H,E
   EC35 06 00         [ 7] 1259     LD  B,#0
   EC37 C3 38 EA      [10] 1260     JP  CTRLM0
                           1261 ;
   EC3A 2A 4C EE      [16] 1262 RIGHT:  LD  HL,(CURX)
   EC3D 2C            [ 4] 1263     INC L
   EC3E 7D            [ 4] 1264     LD  A,L
   EC3F FE 20         [ 7] 1265     CP  #0x20
   EC41 38 0F         [12] 1266     JR  C,CURST2
   EC43 2E 00         [ 7] 1267     LD  L,#0
   EC45 24            [ 4] 1268 DOWNLB: INC H
   EC46 7C            [ 4] 1269     LD  A,H
   EC47 FE 10         [ 7] 1270     CP  #16
   EC49 38 07         [12] 1271     JR  C,CURST2
   EC4B E5            [11] 1272     PUSH    HL
   EC4C CD 6E EA      [17] 1273     CALL    SCRLUP
   EC4F E1            [10] 1274     POP HL
   EC50 26 0F         [ 7] 1275     LD  H,#15
   EC52 22 4C EE      [16] 1276 CURST2: LD  (CURX),HL
   EC55 C9            [10] 1277     RET
                           1278 ;
   EC56 2A 4C EE      [16] 1279 UP: LD  HL,(CURX)
   EC59 18 C7         [12] 1280     JR  UPCUR
                           1281 ;
   EC5B 2A 4C EE      [16] 1282 DOWN:   LD  HL,(CURX)
   EC5E 18 E5         [12] 1283     JR  DOWNLB
                           1284 ;
   EC60 3A 6A EF      [13] 1285 CTRLG:  LD  A,(BELLFG)
   EC63 EE 01         [ 7] 1286     XOR #1
   EC65 32 6A EF      [13] 1287     LD  (BELLFG),A
   EC68 C3 74 ED      [10] 1288     JP  BELL
                           1289 ;
   EC6B 11 00 00      [10] 1290 CTRLB:  LD  DE,#0
   EC6E 21 17 EC      [10] 1291     LD  HL,#LEFT
   EC71 22 A2 EC      [16] 1292     LD  (CTRLBF+1),HL
   EC74 CD 99 EC      [17] 1293 CTRLB1: CALL    CBFCEK
   EC77 C8            [11] 1294     RET Z
   EC78 38 FA         [12] 1295     JR  C,CTRLB1
   EC7A CD 99 EC      [17] 1296 CTRLB2: CALL    CBFCEK
   EC7D C8            [11] 1297     RET Z
   EC7E 30 FA         [12] 1298     JR  NC,CTRLB2
   EC80 C3 3A EC      [10] 1299     JP  RIGHT
                           1300 ;
   EC83 11 1F 0F      [10] 1301 CTRLF:  LD  DE,#0x0F1F
   EC86 21 3A EC      [10] 1302     LD  HL,#RIGHT
   EC89 22 A2 EC      [16] 1303     LD  (CTRLBF+1),HL
   EC8C CD 99 EC      [17] 1304 CTRLF1: CALL    CBFCEK
   EC8F C8            [11] 1305     RET Z
   EC90 30 FA         [12] 1306     JR  NC,CTRLF1
   EC92 CD 99 EC      [17] 1307 CTRLF2: CALL    CBFCEK
   EC95 C8            [11] 1308     RET Z
   EC96 38 FA         [12] 1309     JR  C,CTRLF2
   EC98 C9            [10] 1310     RET
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 25.
Hexadecimal [16-Bits]



                           1311 ;
   EC99 2A 4C EE      [16] 1312 CBFCEK: LD  HL,(CURX)
   EC9C B7            [ 4] 1313     OR  A
   EC9D ED 52         [15] 1314     SBC HL,DE
   EC9F C8            [11] 1315     RET Z
   ECA0 D5            [11] 1316     PUSH    DE
   ECA1 CD 3A EC      [17] 1317 CTRLBF: CALL    RIGHT
   ECA4 CD EF E9      [17] 1318     CALL    ADRCAL
   ECA7 CD 2A ED      [17] 1319     CALL    AINBC
   ECAA D1            [10] 1320     POP DE
   ECAB FE 30         [ 7] 1321     CP  #'0'        ;030h
   ECAD D8            [11] 1322     RET C
   ECAE FE 3A         [ 7] 1323     CP  #'9'+1      ;03Ah
   ECB0 3F            [ 4] 1324     CCF
   ECB1 D0            [11] 1325     RET NC
   ECB2 FE 41         [ 7] 1326     CP  #'A'        ;041h
   ECB4 D8            [11] 1327     RET C
   ECB5 FE 80         [ 7] 1328     CP  #0x80
   ECB7 38 02         [12] 1329     JR  C,L0BEC
   ECB9 B7            [ 4] 1330     OR  A
   ECBA C9            [10] 1331     RET
   ECBB E6 1F         [ 7] 1332 L0BEC:  AND #0x1F
   ECBD 28 04         [12] 1333     JR  Z,L0BF4
   ECBF FE 1B         [ 7] 1334     CP  #0x1B
   ECC1 3F            [ 4] 1335     CCF
   ECC2 D0            [11] 1336     RET NC
   ECC3 E6 01         [ 7] 1337 L0BF4:  AND #1
   ECC5 C9            [10] 1338     RET
                           1339 ;
   ECC6 16 00         [ 7] 1340 CTRLY:  LD  D,#0        ;★라벨이 빠져있었음
   ECC8 18 02         [12] 1341     JR  CTRLYT
                           1342 ;
   ECCA 16 01         [ 7] 1343 CTRLT:  LD  D,#1
   ECCC 2A 4C EE      [16] 1344 CTRLYT: LD  HL,(CURX)
   ECCF 26 00         [ 7] 1345     LD  H,#0
   ECD1 01 A6 EE      [10] 1346     LD  BC,#TABBUF
   ECD4 09            [11] 1347     ADD HL,BC
   ECD5 72            [ 7] 1348     LD  (HL),D
   ECD6 C9            [10] 1349     RET
                           1350 ;
   ECD7 CD 3A EC      [17] 1351 CTRLI:  CALL    RIGHT
   ECDA 3A 4C EE      [13] 1352     LD  A,(CURX)
   ECDD FE 1F         [ 7] 1353     CP  #31
   ECDF C8            [11] 1354     RET Z
   ECE0 6F            [ 4] 1355     LD  L,A
   ECE1 26 00         [ 7] 1356     LD  H,#0
   ECE3 01 A6 EE      [10] 1357     LD  BC,#TABBUF
   ECE6 09            [11] 1358     ADD HL,BC
   ECE7 7E            [ 7] 1359     LD  A,(HL)
   ECE8 B7            [ 4] 1360     OR  A
   ECE9 28 EC         [12] 1361     JR  Z,CTRLI
   ECEB C9            [10] 1362     RET
                           1363 ;
   ECEC CD 5B ED      [17] 1364 CTRLE:  CALL    ECUYST
   ECEF 63            [ 4] 1365     LD  H,E
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 26.
Hexadecimal [16-Bits]



   ECF0 2E 00         [ 7] 1366     LD  L,#0
   ECF2 18 10         [12] 1367     JR  CTRLEZ
                           1368 ;
   ECF4 CD 69 ED      [17] 1369 CTRLZ:  CALL    CUTBHL
   ECF7 3E 10         [ 7] 1370     LD  A,#16
   ECF9 93            [ 4] 1371     SUB E
   ECFA 47            [ 4] 1372     LD  B,A
   ECFB 23            [ 6] 1373     INC HL
   ECFC 36 00         [10] 1374 CTRLZ1: LD  (HL),#0
   ECFE 23            [ 6] 1375     INC HL
   ECFF 10 FB         [13] 1376     DJNZ    CTRLZ1
   ED01 21 00 10      [10] 1377     LD  HL,#0x1000
   ED04 CD F2 E9      [17] 1378 CTRLEZ: CALL    ADRCA2
   ED07 0B            [ 6] 1379     DEC BC
   ED08 C5            [11] 1380     PUSH    BC
   ED09 CD EF E9      [17] 1381     CALL    ADRCAL
   ED0C E1            [10] 1382     POP HL
   ED0D 50            [ 4] 1383     LD  D,B
   ED0E 59            [ 4] 1384     LD  E,C
   ED0F B7            [ 4] 1385     OR  A
   ED10 ED 52         [15] 1386     SBC HL,DE
   ED12 23            [ 6] 1387     INC HL
   ED13 16 00         [ 7] 1388     LD  D,#0        ;구버전,신버전
   ED15 3A 07 EF      [13] 1389     LD  A,(COLORF)
   ED18 E6 03         [ 7] 1390     AND #0x03
   ED1A 5F            [ 4] 1391     LD  E,A
   ED1B ED 51         [12] 1392 CTRLZE: OUT (C),D
   ED1D CB D8         [ 8] 1393     SET 3,B
   ED1F ED 59         [12] 1394     OUT (C),E
   ED21 CB 98         [ 8] 1395     RES 3,B
   ED23 03            [ 6] 1396     INC BC
   ED24 2B            [ 6] 1397     DEC HL
   ED25 7C            [ 4] 1398     LD  A,H
   ED26 B5            [ 4] 1399     OR  L
   ED27 20 F2         [12] 1400     JR  NZ,CTRLZE
   ED29 C9            [10] 1401     RET
   ED2A C5            [11] 1402 AINBC:  PUSH    BC      ;INSERT키가 눌리면 그 라인의 TEXT 끝부터 32번
   ED2B ED 78         [12] 1403     IN  A,(C)       ;COLUMN 사이에 BLANK가 있는지 확인
   ED2D E6 7F         [ 7] 1404     AND #0x7F
   ED2F CB D8         [ 8] 1405     SET 3,B
   ED31 ED 48         [12] 1406     IN  C,(C)
   ED33 CB 51         [ 8] 1407     BIT 2,C
   ED35 28 0A         [12] 1408     JR  Z,ALPHA
   ED37 CB 59         [ 8] 1409     BIT 3,C
   ED39 20 12         [12] 1410     JR  NZ,SEMI6
   ED3B E6 0F         [ 7] 1411     AND #0x0F
   ED3D C6 E0         [ 7] 1412     ADD A,#0xE0
   ED3F 18 0A         [12] 1413     JR  AINOK
   ED41 CB 59         [ 8] 1414 ALPHA:  BIT 3,C
   ED43 28 06         [12] 1415     JR  Z,AINOK
   ED45 FE 60         [ 7] 1416     CP  #0x60
   ED47 30 02         [12] 1417     JR  NC,AINOK
   ED49 C6 80         [ 7] 1418     ADD A,#0x80
   ED4B C1            [10] 1419 AINOK:  POP BC
   ED4C C9            [10] 1420     RET
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 27.
Hexadecimal [16-Bits]



   ED4D C1            [10] 1421 SEMI6:  POP BC
   ED4E AF            [ 4] 1422     XOR A
   ED4F C9            [10] 1423     RET 
   ED50 CD 69 ED      [17] 1424 BCUYST: CALL    CUTBHL
   ED53 AF            [ 4] 1425     XOR A
   ED54 BE            [ 7] 1426 LPBNST: CP  (HL)
   ED55 C8            [11] 1427     RET Z
   ED56 2B            [ 6] 1428     DEC HL
   ED57 1D            [ 4] 1429     DEC E
   ED58 20 FA         [12] 1430     JR  NZ,LPBNST
   ED5A C9            [10] 1431     RET
                           1432 ;
   ED5B CD 69 ED      [17] 1433 ECUYST: CALL    CUTBHL
   ED5E 23            [ 6] 1434 ECUYS2: INC HL
   ED5F 1C            [ 4] 1435     INC E
   ED60 7B            [ 4] 1436     LD  A,E
   ED61 FE 10         [ 7] 1437     CP  #16     ;16라인(1페이지)를 통과했는지 체크
   ED63 D0            [11] 1438     RET NC
   ED64 AF            [ 4] 1439     XOR A
   ED65 BE            [ 7] 1440     CP  (HL)
   ED66 20 F6         [12] 1441     JR  NZ,ECUYS2
   ED68 C9            [10] 1442     RET
                           1443 ;
   ED69 3A 4D EE      [13] 1444 CUTBHL: LD  A,(CURY)
   ED6C 5F            [ 4] 1445 ATBHL:  LD  E,A
   ED6D 16 00         [ 7] 1446     LD  D,#0
   ED6F 2A C9 EB      [16] 1447     LD  HL,(LINEFR)
   ED72 19            [11] 1448     ADD HL,DE       ;CURY+LINEFR(01 OR 00)
   ED73 C9            [10] 1449     RET
   ED74                    1450 BELL:   
   ED74 C9            [10] 1451     RET
   ED75 01 00 20      [10] 1452 IO20SB: LD	BC,#0x2000
   ED78 ED 79         [12] 1453 	OUT	(C),A
   ED7A 32 62 EE      [13] 1454 	LD	(IO2000),A
   ED7D C9            [10] 1455 	RET	
   ED7E 79            [ 4] 1456 SCRNBC: LD	A,C		;SCREEN B,C
   ED7F FE 05         [ 7] 1457 	CP	#5
   ED81 D0            [11] 1458 	RET	NC
   ED82 B7            [ 4] 1459 	OR	A
   ED83 21 10 EE      [10] 1460 	LD	HL,#SCRN00
   ED86 28 13         [12] 1461 	JR	Z,SCRNS0
   ED88 78            [ 4] 1462 	LD	A,B
   ED89 06 00         [ 7] 1463 	LD	B,#0
   ED8B 0D            [ 4] 1464 	DEC	C
   ED8C 21 08 EE      [10] 1465 	LD	HL,#SCRNTB
   ED8F CB 11         [ 8] 1466 	RL	C
   ED91 09            [11] 1467 	ADD	HL,BC
   ED92 5E            [ 7] 1468 	LD	E,(HL)
   ED93 23            [ 6] 1469 	INC	HL
   ED94 56            [ 7] 1470 	LD	D,(HL)
   ED95 EB            [ 4] 1471 	EX	DE,HL
   ED96 3D            [ 4] 1472 	DEC	A
   ED97                    1473 PATCH4:	
   ED97 07            [ 4] 1474 	RLCA
   ED98 07            [ 4] 1475 	RLCA
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 28.
Hexadecimal [16-Bits]



   ED99 07            [ 4] 1476 	RLCA
   ED9A 07            [ 4] 1477 	RLCA
   ED9B 5F            [ 4] 1478 SCRNS0: LD	E,A
   ED9C 3A 62 EE      [13] 1479 	LD	A,(IO2000)
   ED9F E6 CF         [ 7] 1480 	AND	#0xCF
   EDA1 B3            [ 4] 1481 	OR	E
   EDA2 CD 75 ED      [17] 1482 	CALL	IO20SB
   EDA5 5E            [ 7] 1483 	LD	E,(HL)
   EDA6 23            [ 6] 1484 	INC	HL
   EDA7 56            [ 7] 1485 	LD	D,(HL)
   EDA8 23            [ 6] 1486 	INC	HL
   EDA9 EB            [ 4] 1487 	EX	DE,HL
   EDAA 22 C9 EB      [16] 1488 	LD	(LINEFR),HL
   EDAD EB            [ 4] 1489 	EX	DE,HL
   EDAE 5E            [ 7] 1490 	LD	E,(HL)
   EDAF 23            [ 6] 1491 	INC	HL
   EDB0 56            [ 7] 1492 	LD	D,(HL)
   EDB1 23            [ 6] 1493 	INC	HL
   EDB2 EB            [ 4] 1494 	EX	DE,HL
   EDB3 22 DF EA      [16] 1495 	LD	(PAGE16+1),HL
   EDB6 EB            [ 4] 1496 	EX	DE,HL
   EDB7 5E            [ 7] 1497 	LD	E,(HL)
   EDB8 23            [ 6] 1498 	INC	HL
   EDB9 56            [ 7] 1499 	LD	D,(HL)
   EDBA 23            [ 6] 1500 	INC	HL
   EDBB EB            [ 4] 1501 	EX	DE,HL
   EDBC 22 90 EA      [16] 1502 	LD	(PAGE11+1),HL
   EDBF 22 E5 EA      [16] 1503 	LD	(PAGE17+1),HL
   EDC2 01 02 00      [10] 1504 	LD	BC,#2		;★이 2줄 4바이트는
   EDC5 09            [11] 1505 	ADD	HL,BC		;INC HL 두 번 2바이트면 가능하다.
   EDC6 7D            [ 4] 1506 	LD	A,L
   EDC7 32 CC EB      [13] 1507 	LD	(PAGE23+1),A
   EDCA EB            [ 4] 1508 	EX	DE,HL
   EDCB 5E            [ 7] 1509 	LD	E,(HL)
   EDCC 23            [ 6] 1510 	INC	HL
   EDCD 56            [ 7] 1511 	LD	D,(HL)
   EDCE 23            [ 6] 1512 	INC	HL
   EDCF EB            [ 4] 1513 	EX	DE,HL
   EDD0 22 A6 EB      [16] 1514 	LD	(CHRSTA),HL
   EDD3 22 FE E9      [16] 1515 	LD	(PAGE05+1),HL
   EDD6 E5            [11] 1516 	PUSH	HL
   EDD7 CB DC         [ 8] 1517 	SET	3,H
   EDD9 22 B6 EB      [16] 1518 	LD	(ATRSTA),HL
   EDDC EB            [ 4] 1519 	EX	DE,HL
   EDDD 5E            [ 7] 1520 	LD	E,(HL)
   EDDE 23            [ 6] 1521 	INC	HL
   EDDF 56            [ 7] 1522 	LD	D,(HL)
   EDE0 23            [ 6] 1523 	INC	HL
   EDE1 EB            [ 4] 1524 	EX	DE,HL
   EDE2 22 B9 EB      [16] 1525 	LD	(CHRLEN),HL
   EDE5 01 E0 FF      [10] 1526 	LD	BC,#0xFFE0
   EDE8 09            [11] 1527 	ADD	HL,BC
   EDE9 22 D9 EA      [16] 1528 	LD	(CHRLE2),HL
   EDEC C1            [10] 1529 	POP	BC
   EDED 09            [11] 1530 	ADD	HL,BC
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 29.
Hexadecimal [16-Bits]



   EDEE 01 1F 00      [10] 1531 	LD	BC,#0x001F
   EDF1 09            [11] 1532 	ADD	HL,BC
   EDF2 22 CC EA      [16] 1533 	LD	(PAGE12+1),HL
   EDF5 CB DC         [ 8] 1534 	SET	3,H
   EDF7 22 D6 EA      [16] 1535 	LD	(PAGE14+1),HL
   EDFA EB            [ 4] 1536 	EX	DE,HL
   EDFB 5E            [ 7] 1537 	LD	E,(HL)
   EDFC 23            [ 6] 1538 	INC	HL
   EDFD 56            [ 7] 1539 	LD	D,(HL)
   EDFE EB            [ 4] 1540 	EX	DE,HL
   EDFF 22 3F EA      [16] 1541 	LD	(PAGE02+1),HL
   EE02 CB DC         [ 8] 1542 	SET	3,H
   EE04 22 4C EA      [16] 1543 	LD	(PAGE04+1),HL
   EE07 C9            [10] 1544 	RET
                           1545 ;
                           1546 ;
                           1547 ;
   EE08 1C EE              1548 SCRNTB: .dw	SCRN01
   EE0A 28 EE              1549 	.dw	SCRN02
   EE0C 34 EE              1550 	.dw	SCRN03
   EE0E 40 EE              1551 	.dw	SCRN04
                           1552 ;
   EE10 29 EF              1553 SCRN00: .dw	LINEF		;LINEFR
   EE12 68 EF              1554 	.dw	LINEF+63	 ;PAGE16
   EE14 3F 00              1555 	.dw	63		 ;PAGE11
   EE16 00 00              1556 	.dw	0x0000		 ;CHRSTA
   EE18 00 08              1557 	.dw	0x0800		 ;CHRLEN
   EE1A 00 02              1558 	.dw	0x0200		 ;PAGE02
                           1559 ;
   EE1C 29 EF              1560 SCRN01: .dw	LINEF		;LINEFR
   EE1E 38 EF              1561 	.dw	LINEF+15	 ;PAGE16
   EE20 0F 00              1562 	.dw	15		 ;PAGE11
   EE22 00 00              1563 	.dw	0x0000		 ;CHRSTA
   EE24 00 02              1564 	.dw	0x0200		 ;CHRLEN
   EE26 00 00              1565 	.dw	0x0000		 ;PAGE02
                           1566 ;
   EE28 39 EF              1567 SCRN02: .dw	LINEF+16	;LINEFR
   EE2A 48 EF              1568 	.dw	LINEF+31	 ;PAGE16
   EE2C 0F 00              1569 	.dw	15		 ;PAGE11
   EE2E 00 02              1570 	.dw	0x0200		 ;CHRSTA
   EE30 00 02              1571 	.dw	0x0200		 ;CHRLEN
   EE32 00 02              1572 	.dw	0x0200		 ;PAGE02
                           1573 ;
   EE34 49 EF              1574 SCRN03: .dw	LINEF+32	;LINEFR
   EE36 58 EF              1575 	.dw	LINEF+47	 ;PAGE16
   EE38 0F 00              1576 	.dw	015		 ;PAGE11
   EE3A 00 04              1577 	.dw	0x0400		 ;CHRSTA
   EE3C 00 02              1578 	.dw	0x0200		 ;CHRLEN
   EE3E 00 04              1579 	.dw	0x0400		 ;PAGE02
                           1580 ;
   EE40 59 EF              1581 SCRN04: .dw	LINEF+48	;LINEFR
   EE42 68 EF              1582 	.dw	LINEF+63	 ;PAGE16
   EE44 0F 00              1583 	.dw	015		 ;PAGE11
   EE46 00 06              1584 	.dw	0x0600		 ;CHRSTA
   EE48 00 02              1585 	.dw	0x0200		 ;CHRLEN
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 30.
Hexadecimal [16-Bits]



   EE4A 00 06              1586 	.dw	0x0600		 ;PAGE02	
                           1587 
   EE4C 00                 1588 CURX:   .db 0       ; cursor X
   EE4D 00                 1589 CURY:   .db 0       ; cursor Y
   EE4E                    1590 OLDBUF: .ds 10      ; keyboard matrix old buffer
   EE58                    1591 NEWBUF: .ds 9       ; keyboard matrix new buffer
   EE61 FF                 1592 NEWMOD: .db 0xFF    ; keyboard matrix mode flag
                           1593 ;
   EE62 00                 1594 IO2000: .db	0		; 2000H I/O data
   EE63 00                 1595 LOCKMD: .db 0       ; LOCK mode
   EE64 00                 1596 POINT1: .db 0       ; key input buffer 
   EE65 00                 1597 POINT2: .db 0       ; key input buffer 
   EE66                    1598 INKYBF: .ds 0x40    ; key input buffer
   EEA6                    1599 TABBUF: .ds 32      ; TAB flag
   EEC6                    1600 KEYBUF: .ds 0x40    
                           1601 ;INKYBF: .ds 0x40    ; key input buffer
                           1602 ;TABBUF: .ds 32      ; TAB flag
   EF06 00                 1603 GMODEF: .db 0
   EF07 00                 1604 COLORF: .db 0       ; Color Flag
   EF08 00                 1605 GCOLOR: .db 0       ; Graphic Color
   EF09                    1606 SCRLBF: .ds 0x20    ; scroll buffer
   EF29                    1607 LINEF:  .ds 65      ; line return flag
   EF6A 00                 1608 BELLFG: .db 0       ;CTRL-G
                           1609 
                           1610     
   EF6B                    1611 FUNBUF: 
   EF6B 04                 1612     .db 4       ; function definition
   EF6C 44 49 52           1613     .ascii  "DIR"
   EF6F 0D                 1614     .db 0x0D
   EF70                    1615     .ds 11
                           1616     
   EF7B 04                 1617     .db 4
   EF7C 45 52 41 20        1618     .ascii  'ERA '
   EF80 00                 1619     .db 0
   EF81                    1620     .ds 11
                           1621 
   EF8C 04                 1622     .db 4
   EF8D 52 45 4E 20        1623     .ascii  'REN '
   EF91 00                 1624     .db 0
   EF92                    1625     .ds 11
                           1626 
   EF9D 05                 1627     .db 5
   EF9E 53 41 56 45 20     1628     .ascii  'SAVE '
   EFA3                    1629     .ds 10
                           1630 
   EFAD 05                 1631     .db 5
   EFAE 54 59 50 45 20     1632     .ascii  'TYPE '
   EFB3                    1633     .ds 10
                           1634 
   EFBD 04                 1635     .db 4
   EFBE 50 49 50 20        1636     .ascii  'PIP '
   EFC2                    1637     .ds 11
                           1638 
   EFCD 05                 1639     .db 5
   EFCE 53 54 41 54 20     1640     .ascii  'STAT '
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 31.
Hexadecimal [16-Bits]



   EFD3                    1641     .ds 11
                           1642 
   EFDE 05                 1643     .db 5
   EFDF 45 44 20           1644     .ascii  'ED '
   EFE2                    1645     .ds 9
                           1646 
   EFEB 04                 1647     .db 4
   EFEC 41 53 4D 20        1648     .ascii  'ASM '
   EFF0                    1649     .ds 11
                           1650 
   EFFB 04                 1651     .db 4
   EFFC 44 44 54 20        1652     .ascii  'DDT '
   F000                    1653     .ds 11
                           1654 ;
   F00B                    1655 KEYDT1: 
   F00B 00 F5              1656     .dw 0xF500      ;8009  No Shift
   F00D 2D 30 3B           1657     .ascii  '-0;'       ;'
   F010 6C 6F              1658     .dw 0x6F6C      ;LO
   F012 39                 1659     .ascii  '9'     ;9
   F013 00 F4              1660     .dw 0xF400      ;8008
   F015 1F                 1661     .db 0x1F
   F016 3A 2F              1662     .ascii  ':/'        ;/
   F018 6B 69              1663     .dw 0x696B      ;KI
   F01A 38                 1664     .ascii  '8'
   F01B 00 F3              1665     .dw 0xF300      ;8007
   F01D 1E 70              1666     .dw 0x701E      ;P
   F01F 2E                 1667     .ascii  '.'     ;★원래는3F(?)였음
   F020 6A 75              1668     .dw 0x756A      ;JU
   F022 37                 1669     .ascii  '7'
   F023 00 F2              1670     .dw 0xF200      ;8006
   F025 40 78              1671     .dw 0x7840      ;X
   F027 2C                 1672     .ascii  ','
   F028 68 79              1673     .dw 0x7968      ;HV
   F02A 36                 1674     .ascii  '6'
   F02B 00 F1              1675     .dw 0xF100      ;8005
   F02D 1D 5D              1676     .dw 0x5D1D
   F02F 6D 67              1677     .dw 0x676D      ;MG
   F031 74 35              1678     .dw 0x3574      ;T5
   F033 00 00              1679     .dw 0x0000      ;8004
   F035 1C 7C              1680     .dw 0x7C1C
   F037 6E 66              1681     .dw 0x666E      ;NF
   F039 72 34              1682     .dw 0x3472      ;R4
   F03B 08 00              1683     .dw 0x0008      ;DEL ;8003
   F03D 1B 5B              1684     .dw 0x5B1B
   F03F 62 64              1685     .dw 0x6462      ;BD
   F041 65 33              1686     .dw 0x3365      ;E3
   F043 16 00              1687     .dw 0x0016      ;LOCK ;8002
   F045 7A 5D              1688     .dw 0x5D7A      ;Z^
   F047 76 73              1689     .dw 0x7376      ;VS
   F049 77 32              1690     .dw 0x3277      ;W2
   F04B 5E 0B              1691     .dw 0x0B5E      ;HOME ;8001
   F04D 20 0D              1692     .dw 0x0D20
   F04F 63 61              1693     .dw 0x6163      ;CA
   F051 71 31              1694     .dw 0x3171      ;Q1
                           1695 ;
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 32.
Hexadecimal [16-Bits]



                           1696 ;
                           1697 ;
   F053                    1698 KEYDT2: 
   F053 00 FA              1699     .dw 0xFA00  ; Shift
   F055 3D 30 2B 4C 4F 29  1700     .ascii  '=0+LO)'
                           1701 
   F05B 00 F9              1702     .dw 0xF900
   F05D 1F                 1703     .db 0x1F
   F05E 2A 3F 4B 49 28     1704     .ascii  '*?KI('
                           1705 
   F063 00 F8              1706     .dw 0xF800
   F065 1E                 1707     .db 0x1E
   F066 50 3E 4A 55        1708     .ascii  'P>JU'
   F06A 27                 1709     .db 0x27
   F06B 00 F7              1710     .dw 0xF700
   F06D 60                 1711     .db 0x60
   F06E 58 3C 48 59 26     1712     .ascii  'X<HY&'
                           1713 
   F073 00 F6              1714     .dw 0xF600
   F075 1D 7D              1715     .dw 0x7D1D
   F077 4D 47 54 25        1716     .ascii  'MGT%'
   F07B 00 00              1717     .dw 0x0000
   F07D 1C 7F              1718     .dw 0x7F1C
   F07F 4E 46 52 24        1719     .ascii  'NFR$'
   F083 12 00              1720     .dw 0x0012      ;INST
   F085 1B 7B              1721     .dw 0x7B1B
   F087 42 44 45 23        1722     .ascii  'BDE#'
   F08B 17 00              1723     .dw 0x0017      ;LOCK
   F08D 5A                 1724     .ascii  'Z'
   F08E 7D                 1725     .db 0x7D
   F08F 56 53 57 22        1726     .ascii  'VSW"'
   F093 7E 0C              1727     .dw 0x0C7E      ;CLR
   F095 20 0D              1728     .dw 0x0D20
   F097 43 41 51 21        1729     .ascii  'CAQ!'  
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 33.
Hexadecimal [16-Bits]

Symbol Table

    .__.$$$.=  2710 L   |     .__.ABS.=  0000 G   |     .__.CPU.=  0000 L
    .__.H$L.=  0000 L   |   3 ACCDIS     EA17 R   |   3 ACCOL1     EA25 R
  3 ACCOL2     EA2C R   |   3 ACCOUS     EA12 R   |   3 ACCOUT     EA12 R
  3 ACCPRT1    EA06 R   |   3 ADRCA2     E9F2 R   |   3 ADRCAL     E9EF R
  3 AINBC      ED2A R   |   3 AINOK      ED4B R   |   3 ALL00      E345 R
  3 ALL01      E364 R   |   3 ALL02      E383 R   |   3 ALL03      E3A2 R
  3 ALLHD1     E3C1 R   |   3 ALLHD2     E4C0 R   |   3 ALPHA      ED41 R
  3 ASCGET1    E784 R   |   3 ASCPRT     E6A8 R   |   3 ATBHL      ED6C R
  3 ATRCLS     EBC0 R   |   3 ATRSTA     EBB6 R   |   3 BCLDD1     EAF8 R
  3 BCLDD2     EB00 R   |   3 BCLDD3     EB13 R   |   3 BCLDDR     EAF0 R
  3 BCLDIR     EA9B R   |   3 BCLDL1     EAA3 R   |   3 BCLDL2     EAAB R
  3 BCLDL3     EABE R   |   3 BCOTOK     EB34 R   |   3 BCOUTA     EB1C R
  3 BCUYST     ED50 R   |     BDOS    =  D406     |   3 BELL       ED74 R
  3 BELLFG     EF6A R   |   3 BFSTSB     E9D3 R   |     BIAS    =  9800 
    BIOS    =  E200     |     BIOSL   =  0300     |   3 BOOT       E612 R
  3 BRKEY      E77D R   |   3 BRKGTR     E7AD R   |   3 BUFCEK     E78E R
  3 BUFSET     E7A0 R   |   3 CBFCEK     EC99 R   |     CCP     =  CC00 
    CDISK   =  0004     |   3 CHGFLG     E605 R   |   3 CHK00      E5BF R
  3 CHK01      E5CF R   |   3 CHK02      E5DF R   |   3 CHK03      E5EF R
  2 CHKDAC2    CB8C R   |   2 CHKDAC3    CB99 R   |   2 CHKDAV0    CBAB R
  2 CHKDAV1    CBC2 R   |   3 CHKHD1     E5FF R   |   3 CHKHD2     E5FF R
  2 CHKRFD1    CB75 R   |   3 CHRLE2     EAD9 R   |   3 CHRLEN     EBB9 R
  3 CHRSTA     EBA6 R   |     CLR02   =  0AD5     |   3 CLR2       EBA5 R
  3 CLRLP      EBAD R   |     CLS     =  1B42     |   3 COLORF     EF07 R
  3 CONIN      E6A3 R   |   3 CONOUT     E6A7 R   |   3 CONST      E69B R
    CPM_BOOT=  CB00     |   3 CTRLB      EC6B R   |   3 CTRLB1     EC74 R
  3 CTRLB2     EC7A R   |   3 CTRLBF     ECA1 R   |   3 CTRLE      ECEC R
  3 CTRLEZ     ED04 R   |   3 CTRLF      EC83 R   |   3 CTRLF1     EC8C R
  3 CTRLF2     EC92 R   |   3 CTRLG      EC60 R   |   3 CTRLI      ECD7 R
  3 CTRLM      EC31 R   |   3 CTRLM0     EA38 R   |   3 CTRLNO     E97E R
  3 CTRLSB     EA11 R   |   3 CTRLT      ECCA R   |   3 CTRLY      ECC6 R
  3 CTRLYT     ECCC R   |   3 CTRLZ      ECF4 R   |   3 CTRLZ1     ECFC R
  3 CTRLZE     ED1B R   |   3 CURSET     EA6A R   |   3 CURSET0    EA61 R
  3 CURST2     EC52 R   |   3 CURX       EE4C R   |   3 CURY       EE4D R
  3 CUTBHL     ED69 R   |   3 DATADDR    E603 R   |   3 DELETE     EB5E R
  3 DELLP1     EB79 R   |   3 DELLP2     EB9B R   |   3 DIRBF      E2C5 R
  3 DOWN       EC5B R   |   3 DOWNLB     EC45 R   |   3 DPBASE     E233 R
  3 DPBLK      E26D R   |     DSKIX   =  F9DF     |   3 ECUYS2     ED5E R
  3 ECUYST     ED5B R   |   3 FDDD       E600 R   |   3 FDDN       E5FF R
  3 FDDS       E602 R   |   3 FDDT       E601 R   |   3 FLASHA     E837 R
  3 FLASHC     E841 R   |   3 FLASHD     E83A R   |   3 FLASHE     E854 R
  3 FLASHF     E7A1 R   |   3 FLASHG     E836 R   |   3 FLASHO     E84C R
  3 FLASHS     E81A R   |   3 FLGET      E7C9 R   |   3 FLGLP1     E90B R
  3 FLGLP2     E921 R   |   3 FLGLP3     E930 R   |   3 FLGLP4     E92E R
  3 FLGLP5     E935 R   |   3 FLGLP6     E939 R   |   3 FLGLP7     E947 R
  3 FLGLP8     E94E R   |   3 FLGLP9     E943 R   |   3 FLGTL2     E88A R
  3 FLGTLP     E864 R   |   3 FLSHGT     E85E R   |   3 FLSHL3     E887 R
  3 FLSHRT     E893 R   |   3 FUNBUF     EF6B R   |   3 FUNCLP     E9CA R
  3 FUNCTN     E9B7 R   |   3 GCOLOR     EF08 R   |   3 GETRET     E79C R
  3 GMODEF     EF06 R   |   3 GOCPM      E67A R   |     GSAVES  =  1BEA 
  3 HOME       E6B2 R   |   3 INKYBF     EE66 R   |   3 INST       EBD8 R
  3 INSTL1     EBF9 R   |   3 IO2000     EE62 R   |   3 IO20SB     ED75 R
    IOBYTE  =  0003     |   3 KEYBFA     E7DD R   |   3 KEYBFD     E7E0 R
  3 KEYBUF     EEC6 R   |   3 KEYDT1     F00B R   |   3 KEYDT2     F053 R
  3 KEYGT      E7B4 R   |   3 KEYSEN     E89C R   |   3 KEYSN0     E89F R
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 34.
Hexadecimal [16-Bits]

Symbol Table

  3 KEYSN1     E8A1 R   |   3 KEYSN2     E8AE R   |   3 KEYSN3     E8B8 R
  3 KEYST2     E7ED R   |   3 KYBFSR     E80F R   |   3 KYBFST     E957 R
  3 L0BEC      ECBB R   |   3 L0BF4      ECC3 R   |   3 L0D28      E84A R
  3 L0D9D      E8BF R   |   3 L0DA5      E8C7 R   |   3 L0DB1      E8D3 R
  3 L0DB9      E8DB R   |   3 L0E4C      E96E R   |   3 LDERR      E2AF R
  3 LEFT       EC17 R   |   3 LINCLS     EBCD R   |   3 LINEF      EF29 R
  3 LINEFR     EBC9 R   |   3 LIST       E6AE R   |   3 LISTST     E707 R
  3 LOAD1      E644 R   |   3 LOAD2      E65F R   |   3 LOCK       EC0E R
  3 LOCKMD     EE63 R   |   3 LOCKST     EC10 R   |   3 LPBNST     ED54 R
    MSIZE   =  003A     |   3 NEWBUF     EE58 R   |   3 NEWCEK     E8DD R
  3 NEWFLG     E903 R   |   3 NEWKYS     E7E7 R   |   3 NEWMOD     EE61 R
  3 NEWPL1     E8E5 R   |   3 NONKEY     E98E R   |   3 NONKEZ     E991 R
  3 NONSET     E9EC R   |   3 NORET      E9EE R   |     NSECTS  =  002C 
  3 OLDBUF     EE4E R   |   3 PAGE02     EA3E R   |   3 PAGE04     EA4B R
  3 PAGE05     E9FD R   |   3 PAGE11     EA8F R   |   3 PAGE12     EACB R
  3 PAGE14     EAD5 R   |   3 PAGE16     EADE R   |   3 PAGE17     EAE4 R
  3 PAGE23     EBCB R   |   3 PATCH4     ED97 R   |   3 POINT1     EE64 R
  3 POINT2     EE65 R   |   3 PRESEC     E606 R   |   3 PRTMSG     E607 R
  3 PUNCHS     E6AF R   |   3 RDLOOP     E73F R   |   2 RDLOOPx    CB5E R
  3 READ       E719 R   |   3 READER     E6B1 R   |   3 REPETF     E879 R
  3 REPETS     E7D9 R   |   3 REPTSW     E7EB R   |   3 RIGHT      EC3A R
  3 SAMECK     E8F3 R   |   3 SAMELP     E8FB R   |   3 SCRHOME    EBD2 R
  3 SCRLBF     EF09 R   |   3 SCRLC1     EA45 R   |   3 SCRLC2     EA55 R
  3 SCRLDW     EAC7 R   |   3 SCRLUP     EA6E R   |   3 SCRN00     EE10 R
  3 SCRN01     EE1C R   |   3 SCRN02     EE28 R   |   3 SCRN03     EE34 R
  3 SCRN04     EE40 R   |   3 SCRNBC     ED7E R   |   3 SCRNS0     ED9B R
  3 SCRNTB     EE08 R   |     SDCOPY  =  0004     |     SDFORMAT=  0005 
    SDREAD  =  0002     |     SDSEND  =  0003     |     SDWRITE =  0001 
  3 SECT1      E713 R   |   3 SECTRAN    E708 R   |     SECTS   =  0032 
  3 SELDSK     E6D2 R   |   3 SELFD      E6B7 R   |   3 SELFD$     E6C3 R
  3 SEMI6      ED4D R   |   3 SEMIOT     EB40 R   |   3 SETDMA     E702 R
  3 SETKY      E98B R   |   3 SETKY2     E99C R   |   3 SETLP1     E7F7 R
  3 SETPL2     E7FE R   |   3 SETPL3     E807 R   |   3 SETPL4     E80B R
  3 SETSEC     E6EA R   |   3 SETSEC0    E6FD R   |   3 SETTRK     E6DB R
  3 SIGNON     E27C R   |     SIZE    =  1900     |   3 TABBUF     EEA6 R
    TICONT  =  0036     |   3 TRANS      E253 R   |   3 UNLOCK     EC14 R
  3 UP         EC56 R   |   3 UPCUR      EC22 R   |   3 WBOOT      E630 R
  3 WBOOTE     E203 R   |   3 WRITE      E74A R   |   3 WRLOOP     E770 R
  3 ZEROR2     E7C6 R   |   3 ZERORT     E7C3 R   |   2 _sd_load   CB3B R
  2 recvdata   CBA0 R   |   2 sendcmd    CB69 R   |   2 senddata   CB71 R
  2 start      CB00 GR

ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 35.
Hexadecimal [16-Bits]

Area Table

   0 _CODE      size    0   flags    0
   1 _HEADER    size    0   flags    8
   2 _HEADER0   size   CF   flags    8
   3 _HEADER1   size  E9B   flags    8

