PORT	EQU  0x1c   ; port of lowest clock bytes
DISPLAY EQU   1     ; stdout
ZERO    EQU  '0'
alpha   EQU  'a'
RET     EQU   13    ; CR goes to start of display
NL      EQU  0x0a   ; new line clears display

		org 0x200
loop:	LD   C,PORT
    	IN A,(C)
        LD E,A
		INC C
    	IN A,(C)
        LD D,A
		LD A,RET
		OUT (DISPLAY),A
		LD A,D
		call outhexa
		LD A,E
		call outhexa
		JP  loop

outhexa:
		PUSH AF
		SRL  A
		SRL  A
		SRL  A
		SRL  A
		CP   10
		JR   C,ok1
		SUB  10
		ADD  A,alpha
		SUB  ZERO
ok1:	ADD  A,ZERO
        OUT (DISPLAY),A
		POP  AF
		AND  15
        CP   10
        JR   C,ok2
        SUB  10
        ADD  A,alpha
        SUB  ZERO
ok2:	ADD  A,ZERO
		OUT (DISPLAY),A
		RET
