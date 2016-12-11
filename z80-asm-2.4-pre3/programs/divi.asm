		;   PUSH HL       ; 11
		;   PUSH BC       ; 11
		;   LD HL,table8  ; 10
		;   LD C,A        ;  4
		;   LD B,0        ;  7
		;   ADD HL,BC     ; 11
		;   LD A,(HL)     ;  7
		;   POP BC        ; 10
		;   POP HL        ; 10
		;   LD  B,A       ;  4
		;   RET           ; 10
;
		org 0x100
msb:		; return in A index+1 of most sigificant bit in A
            ; set A to 0 if A is zero  undefined on return: F
			; time <= 103 ticks (in the case highest set bit is 2)
		OR A ; clear carry
		RLA
		JR  C,b8
		RLA
		JR  C,b7
		RLA
		JR  C,b6
		RLA
		JR  C,b5
		RLA
		JR  C,b4
		RLA
		JR  C,b3
		RLA
		JR  C,b2
		RLA
		ADC A,A
		RET
b8:     LD  A,8
		RET
b7:     LD  A,7
		RET
b6:     LD  A,6
		RET
b5:     LD  A,5
		RET
b4:     LD  A,4
		RET
b3:     LD  A,3
		RET
b2:     LD  A,2
		RET
;
		org 0x130
left8:
            ; maximal 8 bit left shift of  HL
			; shift counter in A
			; on return: if A < 8 then HL <- HL*2^A, A=0 else HL <- HL*256, A-=8
		CP  8
		JR  NC,minus
		CP  4
		JR  C,small
		JR  Z,c4
        CP  6
        JR  C,c5
		LD  A,0
        JR  NZ,c7
;       JR  c6
   		SRL H
		RR  L
		RRA
c7:		SRL H
		RR  L
		RRA
		LD H,L
		LD L,A
		XOR A
		RET
small:	CP  2
        JR  Z,c2
		JR  NC,c3
		CP  1
        JR  Z,c1
		JR  c0
minus:  SUB  8
		LD  H,L
		LD  L,0
		RET
c5:     ADD HL,HL
c4:     ADD HL,HL
c3:     ADD HL,HL
c2:     ADD HL,HL
c1:     ADD HL,HL
c0:     XOR A
        RET
		;
		org 0x170
shift:      ; 16 bit arithmetic left-/right shift
            ; argument in DE  ; signed shift counter in B
			; result in DE   = DE *2^B
			; undefined on return: A
        LD  A,B
		CP  0
		RET Z
		CP 0x80
		JR  C,left
right:  SRA D
        RR  E
		DJNZ  right
		RET
left:	SLA E
		RL  D
		DJNZ  left
		RET
;
        org 0x190
t_power:    ; returns in in A the  two-power  2^A  if 0<=A<8  or  0  if A >= 8
            ; undefined in return: F
			; time <= 110 ticks (in the case A = 7)
		OR  A
		JR  Z,t0
        DEC A
		JR  Z,t1
		DEC A
		JR  Z,t2
		DEC A
		JR  Z,t3
		DEC A
		JR  Z,t4
		DEC A
		JR  Z,t5
		DEC A
		JR  Z,t6
		DEC A
		JR  Z,t7
		XOR A
		RET
t1:     INC A
t0:     INC A
        RET
t2:     LD A,4
        RET
t3:     LD A,8
        RET
t4:     LD A,16
        RET
t5:     LD A,32
        RET
t6:     LD A,64
        RET
t7:     LD A,128
        RET
;
		org 0x1c0
div_8:		; 8 bit division  dividend in A, divisor in C
            ; remainder in A, if error carry flag cleared else quotient in E
            ; undefined on return: B
			; time <= 585 ticks  (255/3)
		LD  B,A
		LD  A,C
		DEC A
		AND C
		JR  Z,twop
      	LD  A,B
		LD  E,0
		CP  C
		RET c
		INC E
     	SUB C
		JR  C,final
		LD  B,E
back:   INC B
        SUB C
		JR  C,inside
        SLA C
		JP  back
inside:	DEC B
        ADD A,C
posi:   DEC B
        JR Z,read2
        SRL C
		SLA E
		INC E
       	SUB C
        JR NC,posi
        SRL C
        DEC B
        JR Z,final
nega:   SLA E
        DEC E
		ADD A,C
        JR C,posi
		SRL C
		DJNZ nega
      	OR A
		JR Z,read2
final:  ADD A,C
        DEC E
read2:  SCF
		RET
twop:   LD A,B
        DEC C
		AND C
		INC C
		RET Z  ; carry cleared by AND
		LD E,B
rep:	SRL C
		RET c
		SRL E
		JR  rep
;
		org 0x210
dvi_16:		; 16 bit division  dividend in HL, divisor in DE
            ; remainder in HL, if error carry flag cleared else quotient in BC
            ; undefined on return: A
			; time <= 1940 ticks  (2^16-1)/3
        LD  A,D
		OR  E
		RET Z
; check for two-power divisor
		LD  B,D
		LD  C,E
		DEC BC
		LD  A,B
		AND D
		JR  NZ,normal
		LD  A,C
		AND E
		JR  NZ,normal
		LD  B,H
		LD  C,L
		DEC DE
		LD  A,H
		AND D
		LD  H,A
		LD  A,L
		AND E
		LD  L,A
		INC DE
next:	SRL D
		RR  E
		RET C
		SRL B
		RR  C
		JP  next
normal: XOR A
        LD  B,A
		INC A
        LD  C,A
        SBC HL,DE
		JR  C,ready0
adjust: INC A
        SBC HL,DE
		JR  C,ok
		SLA E
		RL  D
		JP  adjust
;       now  A such that holds:
;          dividend - 2^(A-1)*divisor < 0  and  dividend - 2^(A-2)*divisor >= 0
;       now  HL -= 2^(A-1) * divisor   and   DE *= 2^(A-2)
;       now  always holds: |dividend - BC * 2^(A-1)*divisor| < 2^(A-1)*divisor
;       and  always        HL = dividend - BC * 2^(A-1)*divisor
ok:     DEC A
        ADD HL,DE
positiv:
        DEC A
		JR  Z,ready2
		SRL D
		RR  E
        SLA C
        RL  B
		INC BC
        SBC HL,DE
		JR  NC,positiv
negativ:
		SRL D
		RR  E
        DEC A
		JR  Z,ready1
        SLA C
        RL  B
		DEC BC
        ADD HL,DE
		JR  NC,negativ
        JP  positiv
ready1: LD  A,H
        OR  L
		JR  Z,ready2
ready0: ADD HL,DE
        DEC BC
ready2: SCF
 		RET
;
		org 0x280  ; upto 0x3b7
div_32:		; 32 bit division  dividend in HL',HL, divisor in DE',DE
            ; remainder in HL',HL, if error carry flag cleared else
			; quotient in BC',BC
            ; undefined on return: A,A',IX
			; time <= 6649 ticks  (2^32-1)/1
			EXX
			LD  A,D
			CP  0
			JR  Z,over1
			CALL msb
            SUB 9
			CPL
			EXX
			JP found
over1:
			LD  A,E
			CP  0
			JR  Z,over2
			CALL msb
            SUB 17
			CPL
			EXX
			JP found
over2:
			EXX
			LD  A,D
			CP  0
			JR  Z,over3
			CALL msb
			SUB 25
			CPL
			JP found
over3:		
            LD  A,E
			CP  0
			RET Z
			CALL msb
			SUB 33
			CPL
found:		
            LD  C,A
			EXX
			LD  A,H
			CP  0
			JR  Z,next1
			CALL msb
			SUB 9
			CPL
			EXX
			JP finded
next1:
			LD  A,L
			CP  0
			JR  Z,next2
			CALL msb
			SUB 17
			CPL
			EXX
			JP finded
next2:
			EXX
			LD  A,H
			CP  0
			JR  Z,next3
			CALL msb
			SUB 25
			CPL  
			JP finded
next3:		
            LD  A,L
			CP  0
			JP  Z,zero
			CALL msb
			SUB 33
			CPL
finded:   		
            LD  B,A
			LD  A,C
            SUB B
			JP  C,zero
;           now C holds shift count for divisor
;           now B holds shift count for dividend
;           now A holds difference of most significant bit positions
;           so we get:  quotient < 2^(A+1) ; at most A+1 bits long
            LD B,A
			EX AF,AF'
            LD A,B
			SUB 8
			JR C,shi
			LD B,A
			LD A,D
			EXX
			LD D,E
			LD E,A
			EXX
			LD D,E
			LD E,0
			LD A,B
			SUB 8
			JR C,shi
			LD B,A
			LD A,D
			EXX
			LD D,E
			LD E,A
			EXX
			LD D,E
			LD A,B
			SUB 8
			JR C,shi
			LD B,A
			LD A,D
			EXX
			LD D,E
			LD E,A
			EXX
shi:        XOR A
            CP B
			JR Z,each
every:		SLA E
			RL  D
			EXX
			RL  E
			RL  D
            EXX
			DJNZ  every
each:       ;
            LD  B,A
            LD  C,A
			EXX
            LD  B,A
            LD  C,A
;  now DE',DE are HL',HL adjusted (have same highest bit set)
;  BC',BC  holds the to-calculate quotient and A' holds the shift difference
;  and we are in the  '-register set active
            LD  IX,loop0
loop0:       LD  A,H
			CP  D
			JR  C,go_on
			JR  NZ,do_it
			OR  A
			JR  NZ,loop1
            LD  IX,loop1
loop1:      LD  A,L
			CP  E
			JR  C,go_on
			JR  NZ,do_it
			OR  A
			JR  NZ,loop2
            LD  IX,loop2
loop2:		EXX
            LD  A,H
			CP  D
			EXX
			JR  C,go_on
			JR  NZ,do_it
			OR  A
			JR  NZ,loop3
            LD  IX,loop3
loop3:		EXX
            LD  A,L
			CP  E
			EXX
			JR  C,go_on
do_it:      EXX
            INC C
			SBC HL,DE
			EXX
			SBC HL,DE
go_on:      EX  AF,AF'
            OR  A
			JR  Z,finish 
			DEC A
			EX  AF,AF'
			SRL D
			RR  E
			EXX
			RR  D
			RR  E
			SLA C
			RL  B
			EXX
			RL  C
			RL  B
            JP (IX)
zero:       XOR A
            LD  B,A
            LD  C,A
			EXX
            LD  B,A
            LD  C,A
finish:		EXX
			SCF
			RET
;
            org 0x400
div_64:		; 64 bit division  dividend in IY,HL,HL',IX
            ; divisor in DE,BC,DE',BC'
            ; remainder in IY,HL,HL',IX if error carry flag cleared else
            ; quotient in DE(highest), BC(high), DE'(low), BC'(lowest)
            ; needs 4*2=8 bytes on stack
			LD  A,D
			CP  0
			JR  Z,sor1
			CALL msb
            SUB 9
			CPL
			JP dsor
sor1:
			LD  A,E
			CP  0
			JR  Z,sor2
			CALL msb
            SUB 17
			CPL
			JP dsor
sor2:
			LD  A,B
			CP  0
			JR  Z,sor3
			CALL msb
            SUB 25
			CPL
			JP dsor
sor3:
			LD  A,C
			CP  0
			JR  Z,sor4
			CALL msb
            SUB 33
			CPL
			JP dsor
sor4:
            EXX
			LD  A,D
			CP  0
			JR  Z,sor5
			CALL msb
            SUB 41
			CPL
            EXX
			JP dsor
sor5:
			LD  A,E
			CP  0
			JR  Z,sor6
			CALL msb
            SUB 49
			CPL
            EXX
			JP dsor
sor6:
			LD  A,B
			CP  0
			JR  Z,sor7
			CALL msb
            SUB 57
			CPL
            EXX
			JP dsor
sor7:
			LD  A,C
			CP  0
			RET  Z
			CALL msb
            SUB 65
			CPL
            EXX
dsor:		
            EX AF,AF'
            PUSH IY
            POP  AF
			CP  0
			JR  Z,end1
			CALL msb
            SUB 9
			CPL
			JP dend
end1:
            PUSH IY
            DEC  SP
            POP  AF
            INC  SP
			CP  0
			JR  Z,end2
			CALL msb
            SUB 17 
			CPL
			JP dend
end2:
            LD  A,H
			CP  0
			JR  Z,end3
			CALL msb
            SUB 25 
			CPL
			JP dend
end3:
            LD  A,L
			CP  0
			JR  Z,end4
			CALL msb
            SUB 33 
			CPL
			JP dend
end4:
            EXX
            LD  A,H
			CP  0
			JR  Z,end5
			CALL msb
            SUB 41 
			CPL
            EXX
			JP dend
end5:
            LD  A,L
			CP  0
			JR  Z,end6
			CALL msb
            SUB 49 
			CPL
            EXX
			JP dend
end6:
            EXX
            PUSH IX
            POP  AF
			CP  0
			JR  Z,end7
			CALL msb
            SUB 57
			CPL
			JP dend
end7:
            PUSH IX
            DEC  SP
            POP  AF
            INC  SP
			CP  0
			JP  Z,null
			CALL msb
            SUB 65 
            CPL
dend:
;           now A' holds shift count for divisor
;           and A holds shift count for dividend
            PUSH BC
            LD B,A
            EX AF,AF'
			LD C,A
            SUB B
			JP  C,null_pop
            LD  B,A
            PUSH BC   ; divisor-dividend in B  and  shift count of divisor in C
            EX AF,AF' ; now A hold shift count for dividend
            SUB 32
            JR C,shi_d1
            EXX
            PUSH HL
            LD HL,0
            EXX
            POP IY
            PUSH IX
            LD IX,0
            POP HL
            SUB 32
shi_d1:     ADD A,16
			JR NC,shi_d2
            PUSH HL
            POP IY
            EXX
            PUSH HL
            EXX
            POP HL
            PUSH IX
            EXX
            POP HL
            EXX
            LD IX,0
            SUB 16
shi_d2:     ADD A,8
            JR NC,shi_d3
            PUSH HL
            PUSH IY
            POP HL
            POP BC
            LD  H,L
            LD  L,B
            PUSH HL
            POP  IY
            EXX
            PUSH HL
            EXX  
            POP  HL
            EX AF,AF'
            LD   A,L
            LD   L,H
            LD   H,C
            EXX
            PUSH IX
            PUSH IX
            POP  HL
            LD   L,H
            LD   H,A
            EXX
            POP  BC
            LD   B,C
            LD   C,0
            PUSH BC
            POP  IX
            EX AF,AF'
            SUB 8
shi_d3:     ADD A,8
            JR Z,alright
            LD B,A
            XOR A
shi_dend:   ADD IX,IX
			EXX
			ADC HL,HL
            EXX
			ADC HL,HL
            ADC A,A
            ADD IY,IY
            CP 0
            JR Z,not
            INC IY
            XOR A
not:        DJNZ shi_dend
alright:    POP  BC
            LD  A,B  ; divisor-dividend
            EX AF,AF'
            LD  A,C  ; shift count of divisor
            POP BC
			PUSH  AF ; save shift count of divisor
			INC   SP
			EX AF,AF'
			PUSH  AF ; save shift count difference
			INC   SP
			EX AF,AF'
            SUB 32
            JR C,shi_r1
            EXX
            PUSH DE
            PUSH BC
            LD DE,0
            LD BC,0
            EXX
            POP BC
            POP DE
            SUB 32
shi_r1:     ADD A,16
			JR NC,shi_r2
            LD D,B
            LD E,C
            EXX
            PUSH DE
            EXX
            POP BC
            EXX
            LD D,B
            LD E,C
            LD BC,0
            EXX
            SUB 16
shi_r2:     ADD A,8
			JR NC,shi_r3
            LD D,E
            LD E,B
            LD B,C
            LD C,A
            EXX
            LD A,D
            LD D,E
            LD E,B
            LD B,C
            EX AF,AF'
            LD C,A
            EXX
            LD A,C
            EX AF,AF'
            LD C,A
            EXX
            LD A,C
            EX AF,AF'
            LD C,0
            EXX
            SUB 8
shi_r3:     ADD A,8
            JR Z,allrig
shi_dsor:   EXX
            SLA C
            RL  B
            RL  E
            RL  D
            EXX
            RL  C
            RL  B
            RL  E
            RL  D
			DEC A
            JR NZ,shi_dsor
allrig:     
;  now dividend and divisor are both left adjusted (highest 63-th bit set)
;  A' holds the shift difference
			LD A,D
			CPL
			LD D,A
			LD A,E
			CPL
			LD E,A
			INC DE
			EXX
			LD A,B
			CPL
			LD B,A
			LD A,C
			CPL
			LD C,A
			INC BC
			EXX
;			JR Z=BC, lowernocompl
;
repeat:
			PUSH IY
			ADD IY,DE
			JR C,subtr1
			JR NZ,non1
			SBC HL,BC
			JR C,non2
			JR NZ,subtr2
			EXX
			SBC HL,DE
			JR C,non3
			JR NZ,subtr3
			ADD IX,BC
			JR C,subtr4
			JR NZ,non4
			JR subtr4
;
subtr1:     EXX
            ADD IX,BC
            CCF
			SBC HL,DE
			EXX
			SBC HL,BC
			JR  NC,sub_ok
			DEC IY
			JR  sub_ok
subtr2:     EXX
            ADD IX,BC
            CCF
			SBC HL,DE
            JR NC,subtr4 
            EXX
			DEC HL
			JR  sub_ok
subtr3:     ADD IX,BC
            JR C,subtr4 
			DEC HL
subtr4:     EXX 
sub_ok:     POP AF
sure:
			EX AF,AF'
			OR A
			JR Z,final1
			DEC A
			EX AF,AF'
			XOR A
            ADD IX,IX
			INC IX
            JR  common
;
non4:       EX DE,HL
            LD A,B
            CPL
			LD D,A
			LD A,C
            CPL
			LD E,A
			INC DE
			ADD IX,DE
			EX DE,HL
			XOR A
            LD H,A
            LD L,A
non3:       ADD HL,DE
			EXX
non2:       ADD HL,BC
non1:       POP IY
;
			EX AF,AF'
			OR A
			JR Z,final0
			DEC A
			EX AF,AF'
			XOR A
            ADD IX,IX
; 
common:		EXX
			ADC HL,HL
            EXX
			ADC HL,HL
            ADC A,A
            ADD IY,IY
            DEC A
            JR NZ,nocar
            INC IY
			CPL
nocar:      CPL              ; still carry from IY shift
            JP NC, repeat
            ADD IY,DE
            EXX
            ADD IX,BC
            CCF
			SBC HL,DE
			EXX
			SBC HL,BC
			JR  NC,sure
			DEC IY
			JR  sure
;
final1:     SCF
final0:     LD A,0
            ADC A,A
            PUSH IY
            POP DE
			LD  B,H
			LD  C,L
			EXX
			LD  D,H
			LD  E,L
			PUSH IX
			POP BC
			EXX
            ADD IX,IX
            DEC A
			JR NZ,oooo
			INC IX
			CPL
oooo:       CPL
            EXX
			ADC HL,HL
            EXX
			ADC HL,HL
            ADC A,A
            ADD IY,IY
            CP 0
			JR Z,nono
			INC IY
nono:       ;
            POP AF ; now A holds shift count of divisor
			PUSH AF
			CP 32
			JR C,no32
			SUB 32
			PUSH DE
			PUSH BC
			LD D,0
			LD E,D
			LD B,D
			LD C,D
			EXX
			POP BC
			POP DE
			EXX
no32:       CP 16
            JR C,no16
			SUB 16
			PUSH BC
			EXX
			LD C,E
			LD B,D
			POP DE
			EXX
			LD C,E
			LD B,D
			LD D,0
			LD E,D
no16:       CP 8
            JR C,no8
			SUB 8
            EX AF,AF'
            LD A,C
            EXX
			LD C,B
			LD B,E
			LD E,D
			LD D,A
			EX AF,AF'
			EXX
			LD C,B
			LD B,E
			LD E,D
			LD D,0
no8:        CP 0
            JR Z,remainder
remain:		SRL D
			RR  E
			RR  B
			RR  C
			EXX
			RR  D
			RR  E
			RR  B
			RR  C
			EXX
			DEC A
			JR NZ,remain
remainder:  PUSH IX
            PUSH DE
			POP IX
			POP DE
			LD A,H
			LD H,B
			LD B,A
			LD A,L
			LD L,E
			LD E,A
			EXX
			LD A,H
			LD H,B
			LD B,A
			LD A,L
			LD L,E
			LD E,A
            PUSH IX
            PUSH BC
			POP IX
			POP BC
			EXX
            DEC SP
            POP AF ; now A holds shift difference and remainder in IY,HL,HL',IX
			INC SP
			SUB 31
			CPL
			INC A  ; the A highest bits in DE,BC,DE',BC' must still be cleared
			CP 8
			JR NC,weit1
			SUB 9
			CPL
			call t_power
			DEC A
            AND D
			LD D,A
			SCF
			RET
weit1:		SUB 8
			LD D,0
			CP 8
			JR NC,weit2
			SUB 9
			CPL
			call t_power
			DEC A
            AND E
			LD E,A
			SCF
            RET
weit2:      SUB 8
			LD E,0
			CP 8
			JR NC,weit3
			SUB 9
			CPL
			call t_power
			DEC A
            AND B
			LD B,A
			SCF
			RET
weit3:		SUB 8
			LD B,0
			CP 8
			JR NC,weit4
			SUB 9
			CPL
			call t_power
			DEC A
            AND C
			LD C,A
			SCF
			RET
weit4:		SUB 8
            LD C,0
            EXX
			CP 8
			JR NC,weit5
			SUB 9
			CPL
			call t_power
			DEC A
            AND D
			LD D,A
			SCF
			RET
weit5:		SUB 8
            LD D,0
			CP 8
			JR NC,weit6
			SUB 9
			CPL
			call t_power
			DEC A
            AND E
			LD E,A
			SCF
            RET
weit6:      SUB 8
			LD E,0
			CP 8
			JR NC,weit7
			SUB 9
			CPL
			call t_power
			DEC A
            AND B
			LD B,A
			SCF
			RET
weit7:		SUB 8
			LD B,0
;			CP 8
;			JR NC,weit8
			SUB 9
			CPL
			call t_power
			DEC A
            AND C
			LD C,A
			SCF
			RET
null_pop:   POP BC
null:       XOR A
            LD D,A
            LD E,A
            LD B,A
            LD C,A
			EXX
            LD D,A
            LD E,A
            LD B,A
            LD C,A
			EXX
			SCF
			RET
; 
tt_power:    ; returns in in A the  two-power  2^A  if 0<=A<8  or  0  if A >= 8
            ; undefined in return: F
			; time = 71 ticks (in the case A < 8)
			CP 8
			JR NC,out_shifted
		    PUSH HL
			LD HL,table
			OR L
			LD L,A
			LD A,(HL)
			POP HL
			RET
out_shifted XOR A
            RET
			ALIGN 3
table:      defb  1,2,4,8,10h,20h,40h,80H
