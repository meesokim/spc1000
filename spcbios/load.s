;==============================================================================
;   File loading by using tape FIB for SPC-1000
;
;           load.s
;                                   By meeso.kim
;==============================================================================

	.module	load
    .area   _CODE
	.area   HOME
	.area   XSEG
	.area   PSEG
    .area  _HEADER  (ABS)
	.org  0x0104
	.db  0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,0

FSAVE  =  0x0080	
CKSUM  =  0x039e

FILMOD =  0x1396
FILNAM =  0x1397
MTBYTE =  0x13a8
MTADRS =  0x13aa
MTEXEC =  0x13ac
PROTCT =  0x13ae
CKSMF1 =  0x11e3
CKSMF2 =  0x11e5	
MKLEN  =  0x11e7
	
	
FLOAD:	DI
	PUSH	DE
	PUSH	BC
	PUSH	HL
	LD	D,#0xD2
	LD	E,#0xCC
	LD	BC,#FSAVE
	LD	HL,#FILMOD
;
FLOAD1: 
	;CALL	MOTON
	;JP	C,CLOAD5
	CALL	MKRD
;	JP	C,CLOAD5
	CALL	CLOAD
	JP	CLOAD4
;
	.org 0x0134
MLOAD:	DI
	PUSH	DE
	PUSH	BC
	PUSH	HL
	LD	D,#0xD2
	LD	E,#0x53
	LD	HL,(MTBYTE)
	PUSH	HL
	POP	BC
	LD	HL,(MTADRS)
	LD	A,B
	OR	C
	JP	Z,CLOAD4
	JP	FLOAD1
;
CLOAD:	PUSH	DE
	PUSH	BC
	PUSH	HL
	LD	H,#0x02		;원래값은 002h - 소스 수정
;
CLOAD7: 
	LD	BC,#0x0C003
;	LD	A,#14
;	OUT	(C),A
;
CLOAD0: 
;	CALL	EDGE
;	JP	C,CLOAD5
;	CALL	WAITR
	SET 1,A
	OUT (C), A
	IN	A,(C)
	JP	Z,CLOAD0
	LD	D,H
	LD	HL,#00000
	LD	(CKSMF1),HL
	POP	HL
	POP	BC
	PUSH	BC
	PUSH	HL
;
CLOAD1: 
	CALL	VBLOAD
	JP	C,CLOAD5
	LD	(HL),A
	INC	HL
	DEC	BC
	LD	A,B
	OR	C
	JP	NZ,CLOAD1
	LD	HL,(CKSMF1)
	CALL	VBLOAD
	JP	C,CLOAD5
	LD	E,A
	CALL	VBLOAD
	JP	C,CLOAD5
	CP	L
	JP	NZ,CLOAD2
	LD	A,E
	CP	H
	JP	NZ,CLOAD2
;
CLOAD8: XOR	A
;
CLOAD4: POP	HL
	POP	BC
	POP	DE
;	CALL	MOTCH
	EI
	RET
;
CLOAD2: DEC	D
	JP	Z,CLOAD3
	LD	H,D
	JP	CLOAD7
;
CLOAD3: LD	A,#0x01
	SCF
	JP	CLOAD4
;
CLOAD5: LD	A,#0x02
	SCF
	JP	CLOAD4
;
	.org 0x01b9
MVRFY:	
	DI
	PUSH	DE
	PUSH	BC
	PUSH	HL
	LD	HL,(MTBYTE)
	PUSH	HL
	POP	BC
	LD	HL,(MTADRS)
	LD	D,#0xD2
	LD	E,#0x53
	LD	A,B
	OR	C
	JP	Z,CLOAD4
	CALL	#CKSUM
;	CALL	MOTON
	JP	C,CLOAD5
	CALL	MKRD
	JP	C,CLOAD5
	CALL	MVRFY1
	JP	CLOAD4
;
MVRFY1: 
	PUSH	DE
	PUSH	BC
	PUSH	HL
	LD	H,#0x02
;
MVRFYN: 
	LD	BC,#0xC003
;	LD	A,#14
;	OUT	(C),A
;
MVRFY2: 
;	CALL	EDGE
;	JP	C,CLOAD5
;	CALL	WAITR
;	LD	A,040h
;	IN	A,(1)
;	AND	080h
	SET 1, A
	OUT (C), A
	IN A, (C)
	JP	Z,MVRFY2
	LD	D,H
	POP	HL
	POP	BC
	PUSH	BC
	PUSH	HL
;
MVRFY3: 
	CALL	VBLOAD
	JP	C,CLOAD5
	CP	(HL)
	JP	NZ,CLOAD3
	INC	HL
	DEC	BC
	LD	A,B
	OR	C
	JP	NZ,MVRFY3
	LD	HL,(CKSMF2)
	CALL	VBLOAD
	CP	H
	JP	NZ,CLOAD3
	CALL	VBLOAD
	CP	L
	JP	NZ,CLOAD3
	DEC	D
	JP	Z,CLOAD8
	LD	H,D
	JP	MVRFYN
;
;EDGE:
;MVRFY4: LD	A,080h
;	IN	A,(0)		;IN A,(8000H)
;	AND	012h
;	JP	NZ,MVRFY5
;	SCF
;	RET
;
;MVRFY5: LD	A,040h
;	IN	A,(1)		;IN A,(4001H)
;	AND	080h
;	JP	NZ,MVRFY4
;
;MVRFY6: LD	A,080h
;	IN	A,(0)		;IN A,(8001H)
;	AND	012h
;	JP	NZ,MVRFY7
;	SCF
;	RET
;
;MVRFY7: LD	A,040h
;	IN	A,(1)		;IN A,(4001H)
;	AND	080h
;	JP	Z,MVRFY6
;	RET
;
VBLOAD: 
	PUSH	BC
	PUSH	DE
	PUSH	HL
	LD	HL,#0x0800
	LD	BC,#0xC003
;	LD	A,14
;	OUT	(C),A
VBLOD1: 
;	JP	C,VBLOD3
;	CALL	WAITR
;	LD	A,040h
;	IN	A,(1)		;IN A,(4001H)
;	AND	080h
	SET 1, A
	OUT (C), A
	IN A, (C)
	JP	Z,VBLOD2
	PUSH	HL
	LD	HL,(CKSMF1)
	INC	HL
	LD	(CKSMF1),HL
	POP	HL
	SCF
;
VBLOD2: LD	A,L
	RLA
	LD	L,A
	DEC	H
	JP	NZ,VBLOD1
;	CALL	EDGE
	SET 1,A
	OUT (C),A
	IN A,(C)
	LD	A,L
;
VBLOD3: POP	HL
	POP	DE
	POP	BC
	RET
;
MKRD:
	PUSH	BC
	PUSH	DE
	PUSH	HL
	LD	HL,#0x2828
	LD	A,E
	CP	#0xCC
	JP	Z,MKRD1
	LD	HL,#0x1414
;
MKRD1:
	LD	(MKLEN),HL
	LD	BC,#0xC003
MKRD4:
	LD	HL,(MKLEN)
MKRD5:	
;	CALL	EDGE
;	JP	C,MKRD3
;	CALL	WAITR
;	LD	A,040h
;	IN	A,(1)		;IN A,(4001H)
;	AND	080h
	SET 1,A
	OUT (C),A
	IN  A,(C)
	JP	Z,MKRD4
	DEC	H
	JP	NZ,MKRD5
;
MKRD2:
;	CALL	EDGE
;	JP	C,MKRD3
;	CALL	WAITR
;	LD	A,040h
;	IN	A,(1)		;IN A,(4001H)
;	AND	080h
	SET 1, A
	OUT  (C),A
	IN  A,(C)
	JP	NZ,MKRD4
	DEC	L
	JP	NZ,MKRD2
	SET 1, A
	OUT (C),A
;	CALL	EDGE
;
MKRD3:
	POP	HL
	POP	DE
	POP	BC
	RET
;

;MOTON:	PUSH	BC
;	PUSH	DE
;	PUSH	HL
;	LD	BC,#0x4000
;	LD	A,14		;A PORT SELECT
;	OUT	(C),A
;	LD	L,#0x0A
;	LD	BC,06000h
;MOTON1: LD	A,040h
;	IN	A,(1)		;IN A,(4001H)
;	AND	040h
;	JP	NZ,MOTON5
;
;MOTON6: XOR	A
;
;MOTON4: POP	HL
;	POP	DE
;	POP	BC
;	RET
;
;MOTON5: LD	A,(IO6000)
;	RES	1,A
;	OUT	(C),A
;	SET	1,A
;	OUT	(C),A
;	LD	(IO6000),A
;	DEC	L
;	JP	NZ,MOTON1
;	CALL	CR2
;	LD	A,D
;	CP	0D7h
;	JR	Z,MOTON2
;	LD	DE,MBUF6
;	CALL	DEPRT
;	JR	MOTON3
;
;MOTON2: LD	DE,MBUF7
;	CALL	DEPRT
;	LD	DE,MBUF6A
;	CALL	DEPRT
;
;MOTON3: CALL	CR2
;	LD	A,040h
;	IN	A,(1)		;IN A,(4001H)
;	AND	040h
;	JP	Z,MOTON6
;	LD	A,080h
;	IN	A,(0)		;IN A,(8000H)
;	AND	012h
;	JP	NZ,MOTON3
;	SCF
;	JP	MOTON4
;
;MOTCH:	PUSH	AF
;	PUSH	BC
;	PUSH	DE
;	LD	D,00Ah
;	LD	BC,04000h
;	LD	A,14
;	OUT	(C),A
;	LD	BC,06000h
;MOTCH1: LD	A,040h
;	IN	A,(1)		;IN A,(4001H)
;	AND	040h
;	JR	Z,MOTCH2
;	POP	DE
;	POP	BC
;	POP	AF
;	RET
;
;MOTCH2: LD	A,(IO6000)
;	RES	1,A
;	OUT	(C),A
;	SET	1,A
;	OUT	(C),A
;	LD	(IO6000),A
;	DEC	D
;	JP	NZ,MOTCH1
;	POP	DE
;	POP	BC
;	POP	AF
;	RET

	.org 0x0375
WRITEM: 
	.ascii	'WRITING '
	.db	0