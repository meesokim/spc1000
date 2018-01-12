ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 1.
Hexadecimal [16-Bits]



                              1 ;==============================================================================
                              2 ;   File loading by using tape FIB for SPC-1000
                              3 ;
                              4 ;           load.s
                              5 ;                                   By meeso.kim
                              6 ;==============================================================================
                              7 
                              8 	.module	load
                              9     .area   _CODE
                             10 	.area   HOME
                             11 	.area   XSEG
                             12 	.area   PSEG
                             13     .area  _HEADER  (ABS)
   0104                      14 	.org  0x0104
   0104 00 00 00 00 00 00    15 	.db  0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,0
        00 00 00 00 00 00
        00 00 00 00
                             16 
                     0080    17 FSAVE  =  0x0080	
                     039E    18 CKSUM  =  0x039e
                             19 
                     1396    20 FILMOD =  0x1396
                     1397    21 FILNAM =  0x1397
                     13A8    22 MTBYTE =  0x13a8
                     13AA    23 MTADRS =  0x13aa
                     13AC    24 MTEXEC =  0x13ac
                     13AE    25 PROTCT =  0x13ae
                     11E3    26 CKSMF1 =  0x11e3
                     11E5    27 CKSMF2 =  0x11e5	
                     11E7    28 MKLEN  =  0x11e7
                             29 	
                             30 	
   0114 F3            [ 4]   31 FLOAD:	DI
   0115 D5            [11]   32 	PUSH	DE
   0116 C5            [11]   33 	PUSH	BC
   0117 E5            [11]   34 	PUSH	HL
   0118 16 D2         [ 7]   35 	LD	D,#0xD2
   011A 1E CC         [ 7]   36 	LD	E,#0xCC
   011C 01 80 00      [10]   37 	LD	BC,#FSAVE
   011F 21 96 13      [10]   38 	LD	HL,#FILMOD
                             39 ;
   0122                      40 FLOAD1: 
                             41 	;CALL	MOTON
                             42 	;JP	C,CLOAD5
   0122 CD 4E 02      [17]   43 	CALL	MKRD
                             44 ;	JP	C,CLOAD5
   0125 CD 4C 01      [17]   45 	CALL	CLOAD
   0128 C3 91 01      [10]   46 	JP	CLOAD4
                             47 ;
   0134                      48 	.org 0x0134
   0134 F3            [ 4]   49 MLOAD:	DI
   0135 D5            [11]   50 	PUSH	DE
   0136 C5            [11]   51 	PUSH	BC
   0137 E5            [11]   52 	PUSH	HL
   0138 16 D2         [ 7]   53 	LD	D,#0xD2
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 2.
Hexadecimal [16-Bits]



   013A 1E 53         [ 7]   54 	LD	E,#0x53
   013C 2A A8 13      [16]   55 	LD	HL,(MTBYTE)
   013F E5            [11]   56 	PUSH	HL
   0140 C1            [10]   57 	POP	BC
   0141 2A AA 13      [16]   58 	LD	HL,(MTADRS)
   0144 78            [ 4]   59 	LD	A,B
   0145 B1            [ 4]   60 	OR	C
   0146 CA 91 01      [10]   61 	JP	Z,CLOAD4
   0149 C3 22 01      [10]   62 	JP	FLOAD1
                             63 ;
   014C D5            [11]   64 CLOAD:	PUSH	DE
   014D C5            [11]   65 	PUSH	BC
   014E E5            [11]   66 	PUSH	HL
   014F 26 02         [ 7]   67 	LD	H,#0x02		;원래값은 002h - 소스 수정
                             68 ;
   0151                      69 CLOAD7: 
   0151 01 03 C0      [10]   70 	LD	BC,#0x0C003
                             71 ;	LD	A,#14
                             72 ;	OUT	(C),A
                             73 ;
   0154                      74 CLOAD0: 
                             75 ;	CALL	EDGE
                             76 ;	JP	C,CLOAD5
                             77 ;	CALL	WAITR
   0154 A7            [ 4]   78 	AND A
   0155 CB CF         [ 8]   79 	SET 1,A
   0157 ED 79         [12]   80 	OUT (C), A
   0159 ED 78         [12]   81 	IN	A,(C)
   015B CA 54 01      [10]   82 	JP	Z,CLOAD0
   015E 54            [ 4]   83 	LD	D,H
   015F 21 00 00      [10]   84 	LD	HL,#00000
   0162 22 E3 11      [16]   85 	LD	(CKSMF1),HL
   0165 E1            [10]   86 	POP	HL
   0166 C1            [10]   87 	POP	BC
   0167 C5            [11]   88 	PUSH	BC
   0168 E5            [11]   89 	PUSH	HL
                             90 ;
   0169                      91 CLOAD1: 
   0169 CD 20 02      [17]   92 	CALL	VBLOAD
   016C DA A4 01      [10]   93 	JP	C,CLOAD5
   016F 77            [ 7]   94 	LD	(HL),A
   0170 23            [ 6]   95 	INC	HL
   0171 0B            [ 6]   96 	DEC	BC
   0172 78            [ 4]   97 	LD	A,B
   0173 B1            [ 4]   98 	OR	C
   0174 C2 69 01      [10]   99 	JP	NZ,CLOAD1
   0177 2A E3 11      [16]  100 	LD	HL,(CKSMF1)
   017A CD 20 02      [17]  101 	CALL	VBLOAD
   017D DA A4 01      [10]  102 	JP	C,CLOAD5
   0180 5F            [ 4]  103 	LD	E,A
   0181 CD 20 02      [17]  104 	CALL	VBLOAD
   0184 DA A4 01      [10]  105 	JP	C,CLOAD5
   0187 BD            [ 4]  106 	CP	L
   0188 C2 96 01      [10]  107 	JP	NZ,CLOAD2
   018B 7B            [ 4]  108 	LD	A,E
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 3.
Hexadecimal [16-Bits]



   018C BC            [ 4]  109 	CP	H
   018D C2 96 01      [10]  110 	JP	NZ,CLOAD2
                            111 ;
   0190 AF            [ 4]  112 CLOAD8: XOR	A
                            113 ;
   0191 E1            [10]  114 CLOAD4: POP	HL
   0192 C1            [10]  115 	POP	BC
   0193 D1            [10]  116 	POP	DE
                            117 ;	CALL	MOTCH
   0194 FB            [ 4]  118 	EI
   0195 C9            [10]  119 	RET
                            120 ;
   0196 15            [ 4]  121 CLOAD2: DEC	D
   0197 CA 9E 01      [10]  122 	JP	Z,CLOAD3
   019A 62            [ 4]  123 	LD	H,D
   019B C3 51 01      [10]  124 	JP	CLOAD7
                            125 ;
   019E 3E 01         [ 7]  126 CLOAD3: LD	A,#0x01
   01A0 37            [ 4]  127 	SCF
   01A1 C3 91 01      [10]  128 	JP	CLOAD4
                            129 ;
   01A4 3E 02         [ 7]  130 CLOAD5: LD	A,#0x02
   01A6 37            [ 4]  131 	SCF
   01A7 C3 91 01      [10]  132 	JP	CLOAD4
                            133 ;
   01B9                     134 	.org 0x01b9
   01B9                     135 MVRFY:	
   01B9 F3            [ 4]  136 	DI
   01BA D5            [11]  137 	PUSH	DE
   01BB C5            [11]  138 	PUSH	BC
   01BC E5            [11]  139 	PUSH	HL
   01BD 2A A8 13      [16]  140 	LD	HL,(MTBYTE)
   01C0 E5            [11]  141 	PUSH	HL
   01C1 C1            [10]  142 	POP	BC
   01C2 2A AA 13      [16]  143 	LD	HL,(MTADRS)
   01C5 16 D2         [ 7]  144 	LD	D,#0xD2
   01C7 1E 53         [ 7]  145 	LD	E,#0x53
   01C9 78            [ 4]  146 	LD	A,B
   01CA B1            [ 4]  147 	OR	C
   01CB CA 91 01      [10]  148 	JP	Z,CLOAD4
   01CE CD 9E 03      [17]  149 	CALL	#CKSUM
                            150 ;	CALL	MOTON
   01D1 DA A4 01      [10]  151 	JP	C,CLOAD5
   01D4 CD 4E 02      [17]  152 	CALL	MKRD
   01D7 DA A4 01      [10]  153 	JP	C,CLOAD5
   01DA CD E0 01      [17]  154 	CALL	MVRFY1
   01DD C3 91 01      [10]  155 	JP	CLOAD4
                            156 ;
   01E0                     157 MVRFY1: 
   01E0 D5            [11]  158 	PUSH	DE
   01E1 C5            [11]  159 	PUSH	BC
   01E2 E5            [11]  160 	PUSH	HL
   01E3 26 02         [ 7]  161 	LD	H,#0x02
                            162 ;
   01E5                     163 MVRFYN: 
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 4.
Hexadecimal [16-Bits]



   01E5 01 03 C0      [10]  164 	LD	BC,#0xC003
                            165 ;	LD	A,#14
                            166 ;	OUT	(C),A
                            167 ;
   01E8                     168 MVRFY2: 
                            169 ;	CALL	EDGE
                            170 ;	JP	C,CLOAD5
                            171 ;	CALL	WAITR
                            172 ;	LD	A,040h
                            173 ;	IN	A,(1)
                            174 ;	AND	080h
   01E8 CB CF         [ 8]  175 	SET 1, A
   01EA ED 79         [12]  176 	OUT (C), A
   01EC ED 78         [12]  177 	IN A, (C)
   01EE CA E8 01      [10]  178 	JP	Z,MVRFY2
   01F1 54            [ 4]  179 	LD	D,H
   01F2 E1            [10]  180 	POP	HL
   01F3 C1            [10]  181 	POP	BC
   01F4 C5            [11]  182 	PUSH	BC
   01F5 E5            [11]  183 	PUSH	HL
                            184 ;
   01F6                     185 MVRFY3: 
   01F6 CD 20 02      [17]  186 	CALL	VBLOAD
   01F9 DA A4 01      [10]  187 	JP	C,CLOAD5
   01FC BE            [ 7]  188 	CP	(HL)
   01FD C2 9E 01      [10]  189 	JP	NZ,CLOAD3
   0200 23            [ 6]  190 	INC	HL
   0201 0B            [ 6]  191 	DEC	BC
   0202 78            [ 4]  192 	LD	A,B
   0203 B1            [ 4]  193 	OR	C
   0204 C2 F6 01      [10]  194 	JP	NZ,MVRFY3
   0207 2A E5 11      [16]  195 	LD	HL,(CKSMF2)
   020A CD 20 02      [17]  196 	CALL	VBLOAD
   020D BC            [ 4]  197 	CP	H
   020E C2 9E 01      [10]  198 	JP	NZ,CLOAD3
   0211 CD 20 02      [17]  199 	CALL	VBLOAD
   0214 BD            [ 4]  200 	CP	L
   0215 C2 9E 01      [10]  201 	JP	NZ,CLOAD3
   0218 15            [ 4]  202 	DEC	D
   0219 CA 90 01      [10]  203 	JP	Z,CLOAD8
   021C 62            [ 4]  204 	LD	H,D
   021D C3 E5 01      [10]  205 	JP	MVRFYN
                            206 ;
                            207 ;EDGE:
                            208 ;MVRFY4: LD	A,080h
                            209 ;	IN	A,(0)		;IN A,(8000H)
                            210 ;	AND	012h
                            211 ;	JP	NZ,MVRFY5
                            212 ;	SCF
                            213 ;	RET
                            214 ;
                            215 ;MVRFY5: LD	A,040h
                            216 ;	IN	A,(1)		;IN A,(4001H)
                            217 ;	AND	080h
                            218 ;	JP	NZ,MVRFY4
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 5.
Hexadecimal [16-Bits]



                            219 ;
                            220 ;MVRFY6: LD	A,080h
                            221 ;	IN	A,(0)		;IN A,(8001H)
                            222 ;	AND	012h
                            223 ;	JP	NZ,MVRFY7
                            224 ;	SCF
                            225 ;	RET
                            226 ;
                            227 ;MVRFY7: LD	A,040h
                            228 ;	IN	A,(1)		;IN A,(4001H)
                            229 ;	AND	080h
                            230 ;	JP	Z,MVRFY6
                            231 ;	RET
                            232 ;
   0220                     233 VBLOAD: 
   0220 C5            [11]  234 	PUSH	BC
   0221 D5            [11]  235 	PUSH	DE
   0222 E5            [11]  236 	PUSH	HL
   0223 21 00 08      [10]  237 	LD	HL,#0x0800
   0226 01 03 C0      [10]  238 	LD	BC,#0xC003
                            239 ;	LD	A,14
                            240 ;	OUT	(C),A
   0229                     241 VBLOD1: 
                            242 ;	JP	C,VBLOD3
                            243 ;	CALL	WAITR
                            244 ;	LD	A,040h
                            245 ;	IN	A,(1)		;IN A,(4001H)
                            246 ;	AND	080h
   0229 CB CF         [ 8]  247 	SET 1, A
   022B ED 79         [12]  248 	OUT (C), A
   022D ED 78         [12]  249 	IN A, (C)
   022F CA 3C 02      [10]  250 	JP	Z,VBLOD2
   0232 E5            [11]  251 	PUSH	HL
   0233 2A E3 11      [16]  252 	LD	HL,(CKSMF1)
   0236 23            [ 6]  253 	INC	HL
   0237 22 E3 11      [16]  254 	LD	(CKSMF1),HL
   023A E1            [10]  255 	POP	HL
   023B 37            [ 4]  256 	SCF
                            257 ;
   023C 7D            [ 4]  258 VBLOD2: LD	A,L
   023D 17            [ 4]  259 	RLA
   023E 6F            [ 4]  260 	LD	L,A
   023F 25            [ 4]  261 	DEC	H
   0240 C2 29 02      [10]  262 	JP	NZ,VBLOD1
                            263 ;	CALL	EDGE
   0243 CB CF         [ 8]  264 	SET 1,A
   0245 ED 79         [12]  265 	OUT (C),A
   0247 ED 78         [12]  266 	IN A,(C)
   0249 7D            [ 4]  267 	LD	A,L
                            268 ;
   024A E1            [10]  269 VBLOD3: POP	HL
   024B D1            [10]  270 	POP	DE
   024C C1            [10]  271 	POP	BC
   024D C9            [10]  272 	RET
                            273 ;
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 6.
Hexadecimal [16-Bits]



   024E                     274 MKRD:
   024E C5            [11]  275 	PUSH	BC
   024F D5            [11]  276 	PUSH	DE
   0250 E5            [11]  277 	PUSH	HL
   0251 21 28 28      [10]  278 	LD	HL,#0x2828
   0254 7B            [ 4]  279 	LD	A,E
   0255 FE CC         [ 7]  280 	CP	#0xCC
   0257 CA 5D 02      [10]  281 	JP	Z,MKRD1
   025A 21 14 14      [10]  282 	LD	HL,#0x1414
                            283 ;
   025D                     284 MKRD1:
   025D 22 E7 11      [16]  285 	LD	(MKLEN),HL
   0260 01 03 C0      [10]  286 	LD	BC,#0xC003
   0263                     287 MKRD4:
   0263 2A E7 11      [16]  288 	LD	HL,(MKLEN)
   0266                     289 MKRD5:	
                            290 ;	CALL	EDGE
                            291 ;	JP	C,MKRD3
                            292 ;	CALL	WAITR
                            293 ;	LD	A,040h
                            294 ;	IN	A,(1)		;IN A,(4001H)
                            295 ;	AND	080h
   0266 CB CF         [ 8]  296 	SET 1,A
   0268 ED 79         [12]  297 	OUT (C),A
   026A ED 78         [12]  298 	IN  A,(C)
   026C CA 63 02      [10]  299 	JP	Z,MKRD4
   026F 25            [ 4]  300 	DEC	H
   0270 C2 66 02      [10]  301 	JP	NZ,MKRD5
                            302 ;
   0273                     303 MKRD2:
                            304 ;	CALL	EDGE
                            305 ;	JP	C,MKRD3
                            306 ;	CALL	WAITR
                            307 ;	LD	A,040h
                            308 ;	IN	A,(1)		;IN A,(4001H)
                            309 ;	AND	080h
   0273 CB CF         [ 8]  310 	SET 1, A
   0275 ED 79         [12]  311 	OUT  (C),A
   0277 ED 78         [12]  312 	IN  A,(C)
   0279 C2 63 02      [10]  313 	JP	NZ,MKRD4
   027C 2D            [ 4]  314 	DEC	L
   027D C2 73 02      [10]  315 	JP	NZ,MKRD2
   0280 CB CF         [ 8]  316 	SET 1, A
   0282 ED 79         [12]  317 	OUT (C),A
                            318 ;	CALL	EDGE
                            319 ;
   0284                     320 MKRD3:
   0284 E1            [10]  321 	POP	HL
   0285 D1            [10]  322 	POP	DE
   0286 C1            [10]  323 	POP	BC
   0287 C9            [10]  324 	RET
                            325 ;
                            326 
                            327 ;MOTON:	PUSH	BC
                            328 ;	PUSH	DE
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 7.
Hexadecimal [16-Bits]



                            329 ;	PUSH	HL
                            330 ;	LD	BC,#0x4000
                            331 ;	LD	A,14		;A PORT SELECT
                            332 ;	OUT	(C),A
                            333 ;	LD	L,#0x0A
                            334 ;	LD	BC,06000h
                            335 ;MOTON1: LD	A,040h
                            336 ;	IN	A,(1)		;IN A,(4001H)
                            337 ;	AND	040h
                            338 ;	JP	NZ,MOTON5
                            339 ;
                            340 ;MOTON6: XOR	A
                            341 ;
                            342 ;MOTON4: POP	HL
                            343 ;	POP	DE
                            344 ;	POP	BC
                            345 ;	RET
                            346 ;
                            347 ;MOTON5: LD	A,(IO6000)
                            348 ;	RES	1,A
                            349 ;	OUT	(C),A
                            350 ;	SET	1,A
                            351 ;	OUT	(C),A
                            352 ;	LD	(IO6000),A
                            353 ;	DEC	L
                            354 ;	JP	NZ,MOTON1
                            355 ;	CALL	CR2
                            356 ;	LD	A,D
                            357 ;	CP	0D7h
                            358 ;	JR	Z,MOTON2
                            359 ;	LD	DE,MBUF6
                            360 ;	CALL	DEPRT
                            361 ;	JR	MOTON3
                            362 ;
                            363 ;MOTON2: LD	DE,MBUF7
                            364 ;	CALL	DEPRT
                            365 ;	LD	DE,MBUF6A
                            366 ;	CALL	DEPRT
                            367 ;
                            368 ;MOTON3: CALL	CR2
                            369 ;	LD	A,040h
                            370 ;	IN	A,(1)		;IN A,(4001H)
                            371 ;	AND	040h
                            372 ;	JP	Z,MOTON6
                            373 ;	LD	A,080h
                            374 ;	IN	A,(0)		;IN A,(8000H)
                            375 ;	AND	012h
                            376 ;	JP	NZ,MOTON3
                            377 ;	SCF
                            378 ;	JP	MOTON4
                            379 ;
                            380 ;MOTCH:	PUSH	AF
                            381 ;	PUSH	BC
                            382 ;	PUSH	DE
                            383 ;	LD	D,00Ah
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 8.
Hexadecimal [16-Bits]



                            384 ;	LD	BC,04000h
                            385 ;	LD	A,14
                            386 ;	OUT	(C),A
                            387 ;	LD	BC,06000h
                            388 ;MOTCH1: LD	A,040h
                            389 ;	IN	A,(1)		;IN A,(4001H)
                            390 ;	AND	040h
                            391 ;	JR	Z,MOTCH2
                            392 ;	POP	DE
                            393 ;	POP	BC
                            394 ;	POP	AF
                            395 ;	RET
                            396 ;
                            397 ;MOTCH2: LD	A,(IO6000)
                            398 ;	RES	1,A
                            399 ;	OUT	(C),A
                            400 ;	SET	1,A
                            401 ;	OUT	(C),A
                            402 ;	LD	(IO6000),A
                            403 ;	DEC	D
                            404 ;	JP	NZ,MOTCH1
                            405 ;	POP	DE
                            406 ;	POP	BC
                            407 ;	POP	AF
                            408 ;	RET
                            409 
                     3385   410 FILEFG	= 0x3385
                     2208   411 CONTFG  = 0x2208
                     39AE   412 NEW     = 0x39ae
                     7C4E   413 TEXTST  = 0x7c4e
                     7A4D   414 MEMEND	= 0x7a4d
                     39B9   415 CVLOAD  = 0x39b9
                     15C3   416 RUN	    = 0x15c3
                     33E9   417 MEMSET	   =   0x33e9
                     7A49   418 MEMMAX   = 0x7a49
                     0056   419 INITSB	   =   0x0056
                     1B95   420 GRAPH	   =   0x1b95
                     1BEA   421 GSAVES = 0x1bea
                     0AD5   422 CLR2     = 0xad5
                     08A9   423 PSGST  = 0x08a9
                            424 
   0288                     425 _pload:
   0288 32 85 33      [13]  426 	LD	(FILEFG),A
   028B 32 08 22      [13]  427 	LD	(CONTFG),A
   028E                     428 _load:	
   028E CD A9 08      [17]  429 	call PSGST
   0291 CD 14 01      [17]  430 	call FLOAD
   0294 3A 96 13      [13]  431 	ld a, (FILMOD)
   0297 FE 02         [ 7]  432 	cp #2
   0299 20 2E         [12]  433 	jr nz, bload2
   029B ED 56         [ 8]  434 	im 1
   029D FB            [ 4]  435 	ei
   029E 21 00 FF      [10]  436 	LD hl, #0xFF00
   02A1 22 49 7A      [16]  437 	LD  (MEMMAX),HL
   02A4 CD E9 33      [17]  438 	call MEMSET
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 9.
Hexadecimal [16-Bits]



   02A7 21 DC 0A      [10]  439 	ld  HL, #0x0adc
   02AA 36 20         [10]  440 	ld (hl), #0x20
   02AC 2A 86 15      [16]  441 	LD	HL,(#0x1586)
   02AF 22 AA 13      [16]  442 	LD	(MTADRS),HL
   02B2 ED 5B A8 13   [20]  443 	LD	DE,(MTBYTE)
   02B6 19            [11]  444 	ADD	HL,DE	
   02B7 2A 4D 7A      [16]  445 	LD	HL,(MEMEND)
   02BA B7            [ 4]  446 	OR	A
   02BB ED 52         [15]  447 	SBC	HL,DE
   02BD CD 34 01      [17]  448 	CALL MLOAD
   02C0 CD B9 39      [17]  449 	CALL CVLOAD
   02C3 CD D5 0A      [17]  450 	call CLR2
   02C6 C3 C3 15      [10]  451 	JP	RUN
   02C9                     452 bload2:
   02C9 CD 34 01      [17]  453 	CALL MLOAD
   02CC 2A AC 13      [16]  454 	ld hl, (MTEXEC)
   02CF 7C            [ 4]  455 	ld a, h	
   02D0 B5            [ 4]  456 	or l
   02D1 FE 01         [ 7]  457 	cp #1
   02D3 28 B9         [12]  458 	jr z, _load
   02D5 B7            [ 4]  459 	or a
   02D6 20 03         [12]  460 	jr nz, brun
   02D8 2A AA 13      [16]  461 	ld hl, (MTADRS)
   02DB                     462 brun:	
   02DB E9            [ 4]  463 	jp (hl)
                            464 ;	call #NEW
                            465 ;	call #CLR
                            466 ;	LD	SP,(#STRTOP)
                            467 ;	LD	HL,#0xFFFF
                            468 ;	PUSH	HL
                            469 ;	LD	(#SPBUF),SP
                            470 ;	LD	HL,#CINPUT
                            471 ;	LD	(#LODVEC),HL
                            472 ;	LD	A,#0x01
                            473 ;	LD	(#NRLDED),A
                            474 ;	CALL	#BUFCLR
                            475 ;	jp	NMESOK
   0375                     476 	.org 0x0375
   0375                     477 WRITEM: 
   0375 57 52 49 54 49 4E   478 	.ascii	'WRITING '
        47 20
   037D 00                  479 	.db	0
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 10.
Hexadecimal [16-Bits]

Symbol Table

    .__.$$$.=  2710 L   |     .__.ABS.=  0000 G   |     .__.CPU.=  0000 L
    .__.H$L.=  0000 L   |     CKSMF1  =  11E3     |     CKSMF2  =  11E5 
    CKSUM   =  039E     |   6 CLOAD      014C R   |   6 CLOAD0     0154 R
  6 CLOAD1     0169 R   |   6 CLOAD2     0196 R   |   6 CLOAD3     019E R
  6 CLOAD4     0191 R   |   6 CLOAD5     01A4 R   |   6 CLOAD7     0151 R
  6 CLOAD8     0190 R   |     CLR2    =  0AD5     |     CONTFG  =  2208 
    CVLOAD  =  39B9     |     FILEFG  =  3385     |     FILMOD  =  1396 
    FILNAM  =  1397     |   5 FLOAD      0114 R   |   5 FLOAD1     0122 R
    FSAVE   =  0080     |     GRAPH   =  1B95     |     GSAVES  =  1BEA 
    INITSB  =  0056     |     MEMEND  =  7A4D     |     MEMMAX  =  7A49 
    MEMSET  =  33E9     |     MKLEN   =  11E7     |   7 MKRD       024E R
  7 MKRD1      025D R   |   7 MKRD2      0273 R   |   7 MKRD3      0284 R
  7 MKRD4      0263 R   |   7 MKRD5      0266 R   |   6 MLOAD      0134 R
    MTADRS  =  13AA     |     MTBYTE  =  13A8     |     MTEXEC  =  13AC 
  7 MVRFY      01B9 R   |   7 MVRFY1     01E0 R   |   7 MVRFY2     01E8 R
  7 MVRFY3     01F6 R   |   7 MVRFYN     01E5 R   |     NEW     =  39AE 
    PROTCT  =  13AE     |     PSGST   =  08A9     |     RUN     =  15C3 
    TEXTST  =  7C4E     |   7 VBLOAD     0220 R   |   7 VBLOD1     0229 R
  7 VBLOD2     023C R   |   7 VBLOD3     024A R   |   8 WRITEM     0375 R
  7 _load      028E R   |   7 _pload     0288 R   |   7 bload2     02C9 R
  7 brun       02DB R

ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 11.
Hexadecimal [16-Bits]

Area Table

   0 _CODE      size    0   flags    0
   1 HOME       size    0   flags    0
   2 XSEG       size    0   flags    0
   3 PSEG       size    0   flags    0
   4 _HEADER    size    0   flags    8
   5 _HEADER0   size   27   flags    8
   6 _HEADER1   size   76   flags    8
   7 _HEADER2   size  123   flags    8
   8 _HEADER3   size    9   flags    8

