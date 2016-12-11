		org 0x100
mul333:     ; 8x8->8  multiplicands in B and C, result in A
            ; undefined on return: B,C,F
			; time <= 365 ticks
		XOR A
back1:	SRL B
        JR  Z,ready1
		JR	NC,over1
		ADD A,C
over1: 	SLA C
		JR	back1
ready1: RET NC
		ADD A,C
        RET
;
		org 0x140
mul334:     ; 8x8->16  multiplicands in B and C, result in HL
            ; undefined on return: A,B,C,F  
			; time <= 455 ticks
		LD	A,B
		LD  B,0
		LD  L,B
		LD  H,B
back2:	SRL A
		JR	NC,over2
		ADD HL,BC
over2:	RET Z     ; ready , result in HL
    	SLA C
		RL  B
		JP	back2
;
		org 0x180
mul444:  	; 16x16->16  multiplicands in BC and DE, result in HL
            ; undefined on return: A,B,C,D,E,F
			; time <= 1182 ticks
		LD  A,D
		CP  B
		JR  NC,ok3
		LD  D,B
		LD  B,A
		LD  A,E
		LD  E,C
		LD  C,A
ok3:	LD  H,0
		LD  L,0
back3:	SRL D
		RR  E
		JR	NC,over3
		ADD HL,BC
over3:	LD  A,E
		OR  D
		RET Z     ; ready , result in HL
    	SLA C
		RL  B
		JP	back3
;
		org 0x1c0
mul445:		; 16x16->32  multiplicands in BC and DE result in HL'(high),HL(low)
            ; undefined on return: A,B,C,D,E,F,B',C'
			; time <= 1932 ticks
		LD  A,D
		CP  B
		JR  NC,ok4
		LD  D,B
		LD  B,A
		LD  A,E
		LD  E,C
		LD  C,A
ok4:	XOR A
		LD  H,A
		LD  L,A
		EXX
		LD  H,A
		LD  L,A
		LD  B,A
		LD  C,A
back4:	        ; '-registers
		EXX
		SRL D
		RR  E
		JR	NC,over4
		ADD HL,BC
		EXX
		ADC HL,BC
		EXX
over4:  LD  A,E
		OR  D
		RET Z     ; ready , result in HL'(high), HL(low)
    	SLA C
		RL  B
		EXX       ; next '-registers
		RL  C
		RL  B
		JP	back4
;
		org 0x200
mul555:   	; 32x32->32  multiplicands in DE,BC and DE',BC' result in HL',IX(low)
            ; undefined on return: A,B,C,D,E,F,B',C',D',E'
			; time <= 3966 ticks
			; ENTRY back5:  multiplicands in DE,BC and DE',BC'
			;               sum up in  HL', IX
            ; on return result in HL'(high), IX(low)
		XOR A
		EXX
   		LD  H,A
		LD  L,A
        EXX
		LD  IX,0
back5:	SRL D
		RR  E
		RR  B
		RR  C
		EXX
		JR	NC,over5
		ADD IX,BC
		ADC HL,DE
over5:  SLA C
		RL  B
		RL  E
		RL  D
		EXX
		LD  A,D
		OR  E
		JR  NZ,back5
		OR  B
		OR  C
		RET Z     ; ready , result in HL'(high), IX(low)
backb:	SRL B
		RR  C
		EXX
		JR	NC,dont5
		ADD IX,BC
		ADC HL,DE
dont5:  SLA C
		RL  B
		RL  E
		RL  D
		EXX
		LD  A,B
		OR  C
		RET Z     ; ready , result in HL'(high), IX(low)
		JP	backb
;
		org 0x280
mul556:		; 32x32->64  multiplicands in DE,BC and DE',BC'
            ; result in IY(highest), HL(high), HL'(low), IX(lowest)
            ; undefined on return: A,B,C,D,E,F,H,L,B',C',D',E',H',L',F'
			; time <= 5673 ticks
			; ENTRY mul32: multiplicands in DE,BC and DE',BC',
			;              IX(lowest) must(!) be cleared on entry
			;              sum up in  IY(highest), HL(high), HL'(low)
            ; on return result in IY(highest), HL(high), HL'(low), IX(lowest)
   		LD  IY,0
		LD  IX,0
		XOR A
		LD  H,A
		LD  L,A
        EXX
		LD  H,A
		LD  L,A
		EXX
mul32: 	LD  A,C
		OR  B
		OR  E
		OR  D
		RET Z
		EXX
		ADD IX,BC
		XOR A
		LD  B,A
		LD  C,A
back6:  EXX
        SRL D       ; registers
        RR  E
        RR  B
        RR  C
		EXX
        RR  B
        RR  C
		EX  AF,AF'
        ADD IX,IX
		RL  E
		RL  D
		JR	NC,over6 ; '-registers
		ADD HL,BC
		EXX
		ADC HL,BC
		JR NC,dont6
		INC IY
dont6: 	ADD IY,DE
        EXX
		SCF
over6:	EX  AF,AF'   ; '-registers
		JR  NC,back6
		LD  A,D      ; '-registers
		EXX
		EX  AF,AF'   ; D is 0
		RR  D        ; E is 0
		ADD IX,DE    ; IX becomes 0 or 8000H
		EX  AF,AF'
        LD  D,A
		EXX
		LD  A,E      ; '-registers
		LD  E,0
		LD  D,128
		EXX
		LD  E,A
next6:	LD  A,E
        OR  D        ; registers
        RET Z
        SRL B
        RR  C
		EXX
        RR  B
        RR  C
        RR  D
        RR  E
		EXX
		SLA E
		RL  D
		JR	NC,next6
		EXX
		ADD IX,DE
      	ADC HL,BC
		EXX
		ADC HL,BC
		JR  NC,next6
        INC IY
        JP next6
;
		org 0x300
mul666:		; 64x64 multiplicands in DE(highest), BC(high), HL'(low), IX(lowest)
            ; and in IY(highest), HL(high), DE'(low), BC'(lowest)
            ; result in IY(highest), HL(high), HL'(low), IX(lowest)
            ; undefined on return: A,B,C,D,E,F,H,L,B',C',D',E',H',L',F'
			; needs 6*2+2=14 bytes on stack
			; time <= 13861 ticks
		EXX
   		PUSH  BC
   		PUSH  DE
   		PUSH  IX
   		PUSH  HL
		EXX
		PUSH  HL
   		PUSH  IY
		CALL  mul555   ; low factor times high factor
		POP   DE       ; high word
		POP   BC       ; low word 
		EXX
		POP   DE       ; high word
		POP   BC       ; low word
		PUSH  BC
		PUSH  DE       ; SP+4 effectivley
		EXX
        CALL  back5    ; stack untouched (if no interrupt!)
		PUSH  IX
		EXX
		PUSH  HL
		POP   IY        
		EXX
		POP   HL
		EXX
		LD    H,0
		LD    L,0
		LD    IX,0     ; sum ready for multiplication of two lowest double words
		POP   DE
		POP   BC
        EXX
		POP   DE
		POP   BC
        CALL  mul32
		RET
		END
