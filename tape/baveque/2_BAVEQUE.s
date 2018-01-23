	.module	baveque
	.area   _CODE
	.area   HOME
	.area   XSEG
	.area   PSEG
	.area  _HEADER  (ABS)
        .ORG     0x01A3

L01A3:  JR      L01B9

L01A5:  DEC     D
        JP      Z,L01AD
        LD      H,D
        JP      0x0151

L01AD:  LD      A,#01
        SCF
        JP      0x019D

L01B3:  LD      A,#02
        SCF
        JP      0x019D

L01B9:  NOP
        NOP
        XOR     A
        LD      (#0x6BE),A
        LD      A,#0x11
        LD      HL,#0x12C9  
        LD      (L01A3),HL	
        LD      HL,#0x1F7D	; LD A, L / RRA
        LD      (#0x275),HL	; -> 275h
        LD      A,#0x2B       ; DEC HL
        LD      (#0x26F),A	; -> 26Fh
        CALL    0x027
        CALL    0x02A
        JP    	0x02C7

