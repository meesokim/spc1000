input	EQU	 0
ESC   	EQU 27
ctrl    EQU 0x0a
data    EQU 0x0b
curx    EQU '@'
cury    EQU '#'
contr   EQU '$'


loop:	IN  A,(input)
        CP  0
		JR  Z,loop
		CP  ESC
		RET Z
		CP  curx
		JR  Z  overx
		CP  cury
		JR  Z  overy
		CP  contr
		JR  Z  overc
     	EX  AF,AF'
        call  ready
		EX  AF,AF'
		call  send
		JP loop
overx:	IN  A,(input)
        CP  0
        JR  Z,overx
		EX  AF,AF'
        call  ready
		OR  2
		EX  AF,AF'
		call  send
		JP loop
overy:	IN  A,(input)
        CP  0
        JR  Z,overy
		EX  AF,AF'
        call  ready
		OR  4
		EX  AF,AF'
		call  send
		JP loop
overc:	IN  A,(input)
        CP  0
        JR  Z,overc
		EX  AF,AF'
        call  ready
		OR  6
		EX  AF,AF'
		call  send
		JP loop

ready:	IN  A,(ctrl)
		BIT 3,A
		JR  Z,ready     ; wait for ready
		BIT 0,A
		JR  NZ,ready    ; wait for acknowledge
        OR  1
        AND 0xF9        ; data is default
		XOR 0x80
		RET

send:   OUT (data),A
		EX  AF,AF'
		OUT (ctrl),A
		EX  AF,AF'
		RET
