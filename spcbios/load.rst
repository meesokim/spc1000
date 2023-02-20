ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 1.
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
   0122 CD 87 02      [17]   43 	CALL	MKRD
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
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 2.
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
   0169 CD 59 02      [17]   92 	CALL	VBLOAD
   016C DA A4 01      [10]   93 	JP	C,CLOAD5
   016F 77            [ 7]   94 	LD	(HL),A
   0170 23            [ 6]   95 	INC	HL
   0171 0B            [ 6]   96 	DEC	BC
   0172 78            [ 4]   97 	LD	A,B
   0173 B1            [ 4]   98 	OR	C
   0174 C2 69 01      [10]   99 	JP	NZ,CLOAD1
   0177 2A E3 11      [16]  100 	LD	HL,(CKSMF1)
   017A CD 59 02      [17]  101 	CALL	VBLOAD
   017D DA A4 01      [10]  102 	JP	C,CLOAD5
   0180 5F            [ 4]  103 	LD	E,A
   0181 CD 59 02      [17]  104 	CALL	VBLOAD
   0184 DA A4 01      [10]  105 	JP	C,CLOAD5
   0187 BD            [ 4]  106 	CP	L
   0188 C2 96 01      [10]  107 	JP	NZ,CLOAD2
   018B 7B            [ 4]  108 	LD	A,E
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 3.
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
   01D4 CD 87 02      [17]  152 	CALL	MKRD
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
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 4.
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
   01F6 CD 59 02      [17]  186 	CALL	VBLOAD
   01F9 DA A4 01      [10]  187 	JP	C,CLOAD5
   01FC BE            [ 7]  188 	CP	(HL)
   01FD C2 9E 01      [10]  189 	JP	NZ,CLOAD3
   0200 23            [ 6]  190 	INC	HL
   0201 0B            [ 6]  191 	DEC	BC
   0202 78            [ 4]  192 	LD	A,B
   0203 B1            [ 4]  193 	OR	C
   0204 C2 F6 01      [10]  194 	JP	NZ,MVRFY3
   0207 2A E5 11      [16]  195 	LD	HL,(CKSMF2)
   020A CD 59 02      [17]  196 	CALL	VBLOAD
   020D BC            [ 4]  197 	CP	H
   020E C2 9E 01      [10]  198 	JP	NZ,CLOAD3
   0211 CD 59 02      [17]  199 	CALL	VBLOAD
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
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 5.
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
   0259                     233 	.org 0x259 
   0259                     234 VBLOAD: 
   0259 C5            [11]  235 	PUSH	BC
   025A D5            [11]  236 	PUSH	DE
   025B E5            [11]  237 	PUSH	HL
   025C 21 00 08      [10]  238 	LD	HL,#0x0800
   025F 01 03 C0      [10]  239 	LD	BC,#0xC003
                            240 ;	LD	A,14
                            241 ;	OUT	(C),A
   0262                     242 VBLOD1: 
                            243 ;	JP	C,VBLOD3
                            244 ;	CALL	WAITR
                            245 ;	LD	A,040h
                            246 ;	IN	A,(1)		;IN A,(4001H)
                            247 ;	AND	080h
   0262 CB CF         [ 8]  248 	SET 1, A
   0264 ED 79         [12]  249 	OUT (C), A
   0266 ED 78         [12]  250 	IN A, (C)
   0268 CA 75 02      [10]  251 	JP	Z,VBLOD2
   026B E5            [11]  252 	PUSH	HL
   026C 2A E3 11      [16]  253 	LD	HL,(CKSMF1)
   026F 23            [ 6]  254 	INC	HL
   0270 22 E3 11      [16]  255 	LD	(CKSMF1),HL
   0273 E1            [10]  256 	POP	HL
   0274 37            [ 4]  257 	SCF
                            258 ;
   0275 7D            [ 4]  259 VBLOD2: LD	A,L
   0276 17            [ 4]  260 	RLA
   0277 6F            [ 4]  261 	LD	L,A
   0278 25            [ 4]  262 	DEC	H
   0279 C2 62 02      [10]  263 	JP	NZ,VBLOD1
                            264 ;	CALL	EDGE
   027C CB CF         [ 8]  265 	SET 1,A
   027E ED 79         [12]  266 	OUT (C),A
   0280 ED 78         [12]  267 	IN A,(C)
   0282 7D            [ 4]  268 	LD	A,L
                            269 ;
   0283 E1            [10]  270 VBLOD3: POP	HL
   0284 D1            [10]  271 	POP	DE
   0285 C1            [10]  272 	POP	BC
   0286 C9            [10]  273 	RET
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 6.
Hexadecimal [16-Bits]



                            274 ;
   0287                     275 MKRD:
   0287 C5            [11]  276 	PUSH	BC
   0288 D5            [11]  277 	PUSH	DE
   0289 E5            [11]  278 	PUSH	HL
   028A 21 28 28      [10]  279 	LD	HL,#0x2828
   028D 7B            [ 4]  280 	LD	A,E
   028E FE CC         [ 7]  281 	CP	#0xCC
   0290 CA 96 02      [10]  282 	JP	Z,MKRD1
   0293 21 14 14      [10]  283 	LD	HL,#0x1414
                            284 ;
   0296                     285 MKRD1:
   0296 22 E7 11      [16]  286 	LD	(MKLEN),HL
   0299 01 03 C0      [10]  287 	LD	BC,#0xC003
   029C                     288 MKRD4:
   029C 2A E7 11      [16]  289 	LD	HL,(MKLEN)
   029F                     290 MKRD5:	
                            291 ;	CALL	EDGE
                            292 ;	JP	C,MKRD3
                            293 ;	CALL	WAITR
                            294 ;	LD	A,040h
                            295 ;	IN	A,(1)		;IN A,(4001H)
                            296 ;	AND	080h
   029F CB CF         [ 8]  297 	SET 1,A
   02A1 ED 79         [12]  298 	OUT (C),A
   02A3 ED 78         [12]  299 	IN  A,(C)
   02A5 CA 9C 02      [10]  300 	JP	Z,MKRD4
   02A8 25            [ 4]  301 	DEC	H
   02A9 C2 9F 02      [10]  302 	JP	NZ,MKRD5
                            303 ;
   02AC                     304 MKRD2:
                            305 ;	CALL	EDGE
                            306 ;	JP	C,MKRD3
                            307 ;	CALL	WAITR
                            308 ;	LD	A,040h
                            309 ;	IN	A,(1)		;IN A,(4001H)
                            310 ;	AND	080h
   02AC CB CF         [ 8]  311 	SET 1, A
   02AE ED 79         [12]  312 	OUT  (C),A
   02B0 ED 78         [12]  313 	IN  A,(C)
   02B2 C2 9C 02      [10]  314 	JP	NZ,MKRD4
   02B5 2D            [ 4]  315 	DEC	L
   02B6 C2 AC 02      [10]  316 	JP	NZ,MKRD2
   02B9 CB CF         [ 8]  317 	SET 1, A
   02BB ED 79         [12]  318 	OUT (C),A
                            319 ;	CALL	EDGE
                            320 ;
   02BD                     321 MKRD3:
   02BD E1            [10]  322 	POP	HL
   02BE D1            [10]  323 	POP	DE
   02BF C1            [10]  324 	POP	BC
   02C0 C9            [10]  325 	RET
                            326 ;
                            327 
                            328 ;MOTON:	PUSH	BC
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 7.
Hexadecimal [16-Bits]



                            329 ;	PUSH	DE
                            330 ;	PUSH	HL
                            331 ;	LD	BC,#0x4000
                            332 ;	LD	A,14		;A PORT SELECT
                            333 ;	OUT	(C),A
                            334 ;	LD	L,#0x0A
                            335 ;	LD	BC,06000h
                            336 ;MOTON1: LD	A,040h
                            337 ;	IN	A,(1)		;IN A,(4001H)
                            338 ;	AND	040h
                            339 ;	JP	NZ,MOTON5
                            340 ;
                            341 ;MOTON6: XOR	A
                            342 ;
                            343 ;MOTON4: POP	HL
                            344 ;	POP	DE
                            345 ;	POP	BC
                            346 ;	RET
                            347 ;
                            348 ;MOTON5: LD	A,(IO6000)
                            349 ;	RES	1,A
                            350 ;	OUT	(C),A
                            351 ;	SET	1,A
                            352 ;	OUT	(C),A
                            353 ;	LD	(IO6000),A
                            354 ;	DEC	L
                            355 ;	JP	NZ,MOTON1
                            356 ;	CALL	CR2
                            357 ;	LD	A,D
                            358 ;	CP	0D7h
                            359 ;	JR	Z,MOTON2
                            360 ;	LD	DE,MBUF6
                            361 ;	CALL	DEPRT
                            362 ;	JR	MOTON3
                            363 ;
                            364 ;MOTON2: LD	DE,MBUF7
                            365 ;	CALL	DEPRT
                            366 ;	LD	DE,MBUF6A
                            367 ;	CALL	DEPRT
                            368 ;
                            369 ;MOTON3: CALL	CR2
                            370 ;	LD	A,040h
                            371 ;	IN	A,(1)		;IN A,(4001H)
                            372 ;	AND	040h
                            373 ;	JP	Z,MOTON6
                            374 ;	LD	A,080h
                            375 ;	IN	A,(0)		;IN A,(8000H)
                            376 ;	AND	012h
                            377 ;	JP	NZ,MOTON3
                            378 ;	SCF
                            379 ;	JP	MOTON4
                            380 ;
                            381 ;MOTCH:	PUSH	AF
                            382 ;	PUSH	BC
                            383 ;	PUSH	DE
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 8.
Hexadecimal [16-Bits]



                            384 ;	LD	D,00Ah
                            385 ;	LD	BC,04000h
                            386 ;	LD	A,14
                            387 ;	OUT	(C),A
                            388 ;	LD	BC,06000h
                            389 ;MOTCH1: LD	A,040h
                            390 ;	IN	A,(1)		;IN A,(4001H)
                            391 ;	AND	040h
                            392 ;	JR	Z,MOTCH2
                            393 ;	POP	DE
                            394 ;	POP	BC
                            395 ;	POP	AF
                            396 ;	RET
                            397 ;
                            398 ;MOTCH2: LD	A,(IO6000)
                            399 ;	RES	1,A
                            400 ;	OUT	(C),A
                            401 ;	SET	1,A
                            402 ;	OUT	(C),A
                            403 ;	LD	(IO6000),A
                            404 ;	DEC	D
                            405 ;	JP	NZ,MOTCH1
                            406 ;	POP	DE
                            407 ;	POP	BC
                            408 ;	POP	AF
                            409 ;	RET
                            410 
                     3385   411 FILEFG	= 0x3385
                     2208   412 CONTFG  = 0x2208
                     39AE   413 NEW     = 0x39ae
                     7C4E   414 TEXTST  = 0x7c4e
                     7A4D   415 MEMEND	= 0x7a4d
                     39B9   416 CVLOAD  = 0x39b9
                     15C3   417 RUN	    = 0x15c3
                     33E9   418 MEMSET	   =   0x33e9
                     7A49   419 MEMMAX   = 0x7a49
                     0056   420 INITSB	   =   0x0056
                     1B95   421 GRAPH	   =   0x1b95
                     1BEA   422 GSAVES = 0x1bea
                     0AD5   423 CLR2     = 0xad5
                     08A9   424 PSGST  = 0x08a9
                            425 
   02C1                     426 _pload:
   02C1 32 85 33      [13]  427 	LD	(FILEFG),A
   02C4 32 08 22      [13]  428 	LD	(CONTFG),A
   02C7                     429 _load:	
   02C7 CD A9 08      [17]  430 	call PSGST
   02CA CD 14 01      [17]  431 	call FLOAD
   02CD 3A 96 13      [13]  432 	ld a, (FILMOD)
   02D0 FE 02         [ 7]  433 	cp #2
   02D2 20 2B         [12]  434 	jr nz, bload2
   02D4 ED 56         [ 8]  435 	im 1
   02D6 FB            [ 4]  436 	ei
   02D7 21 00 FF      [10]  437 	LD hl, #0xFF00
   02DA 22 49 7A      [16]  438 	LD  (MEMMAX),HL
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 9.
Hexadecimal [16-Bits]



   02DD CD E9 33      [17]  439 	call MEMSET
   02E0 21 DC 0A      [10]  440 	ld  HL, #0x0adc
   02E3 36 20         [10]  441 	ld (hl), #0x20
   02E5 2A 9D 3A      [16]  442 	LD	HL,(#0x3a9d)
   02E8 22 AA 13      [16]  443 	LD	(MTADRS),HL
   02EB ED 5B A8 13   [20]  444 	LD	DE,(MTBYTE)
   02EF 19            [11]  445 	ADD	HL,DE	
   02F0 2A 4D 7A      [16]  446 	LD	HL,(MEMEND)
   02F3 B7            [ 4]  447 	OR	A
   02F4 ED 52         [15]  448 	SBC	HL,DE
   02F6 CD 34 01      [17]  449 	CALL MLOAD
   02F9 CD B9 39      [17]  450 	CALL CVLOAD
                            451 	;call CLR2
   02FC C3 C3 15      [10]  452 	JP	RUN
   02FF                     453 bload2:
   02FF CD 34 01      [17]  454 	CALL MLOAD
   0302 2A AC 13      [16]  455 	ld hl, (MTEXEC)
   0305 7C            [ 4]  456 	ld a, h	
   0306 B5            [ 4]  457 	or l
   0307 FE 01         [ 7]  458 	cp #1
   0309 28 BC         [12]  459 	jr z, _load
   030B B7            [ 4]  460 	or a
   030C 20 03         [12]  461 	jr nz, brun
   030E 2A AA 13      [16]  462 	ld hl, (MTADRS)
   0311                     463 brun:	
   0311 E9            [ 4]  464 	jp (hl)
                            465 ;	call #NEW
                            466 ;	call #CLR
                            467 ;	LD	SP,(#STRTOP)
                            468 ;	LD	HL,#0xFFFF
                            469 ;	PUSH	HL
                            470 ;	LD	(#SPBUF),SP
                            471 ;	LD	HL,#CINPUT
                            472 ;	LD	(#LODVEC),HL
                            473 ;	LD	A,#0x01
                            474 ;	LD	(#NRLDED),A
                            475 ;	CALL	#BUFCLR
                            476 ;	jp	NMESOK
   0375                     477 	.org 0x0375
   0375                     478 WRITEM: 
   0375 57 52 49 54 49 4E   479 	.ascii	'WRITING '
        47 20
   037D 00                  480 	.db	0
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 10.
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
    MEMSET  =  33E9     |     MKLEN   =  11E7     |   8 MKRD       0287 R
  8 MKRD1      0296 R   |   8 MKRD2      02AC R   |   8 MKRD3      02BD R
  8 MKRD4      029C R   |   8 MKRD5      029F R   |   6 MLOAD      0134 R
    MTADRS  =  13AA     |     MTBYTE  =  13A8     |     MTEXEC  =  13AC 
  7 MVRFY      01B9 R   |   7 MVRFY1     01E0 R   |   7 MVRFY2     01E8 R
  7 MVRFY3     01F6 R   |   7 MVRFYN     01E5 R   |     NEW     =  39AE 
    PROTCT  =  13AE     |     PSGST   =  08A9     |     RUN     =  15C3 
    TEXTST  =  7C4E     |   8 VBLOAD     0259 R   |   8 VBLOD1     0262 R
  8 VBLOD2     0275 R   |   8 VBLOD3     0283 R   |   9 WRITEM     0375 R
  8 _load      02C7 R   |   8 _pload     02C1 R   |   8 bload2     02FF R
  8 brun       0311 R

ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180 / ZX-Next / eZ80), page 11.
Hexadecimal [16-Bits]

Area Table

   0 _CODE      size    0   flags    0
   1 HOME       size    0   flags    0
   2 XSEG       size    0   flags    0
   3 PSEG       size    0   flags    0
   4 _HEADER    size    0   flags    8
   5 _HEADER0   size   27   flags    8
   6 _HEADER1   size   76   flags    8
   7 _HEADER2   size   67   flags    8
   8 _HEADER3   size   B9   flags    8
   9 _HEADER4   size    9   flags    8

