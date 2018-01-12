one_seconde: ; usage of registers A,B,DE
             ; return from a call of this routine last exactly 1 second
speed   EQU  2500       ; clock frequency in kHz  (min. 2 kHz)
		LD   DE, speed  ; 10 T
        DEC  DE         ;  6 T
back:   ; 7+13*x-5 + 10 + 26 = 1000
        LD   B,74       ;  7 T
loop1:  DJNZ  loop1     ; 13 T
        ;  waste 10 ticks
        DEC  BC         ;  6 T
        INC  C          ;  4 T
        DEC  DE         ;  6 T
        LD   A,E        ;  4 T
        OR   D          ;  4 T
        JR   NZ,back    ; 12 T
       ; 16-5+7+74*13-5+10+10= 1000 T
        LD   B,74       ;  7 T
loop2:  DJNZ  loop2     ; 13 T
        ; waste 15 ticks
        NEG             ; 8 T
        LD   A,0        ; 7 T
        RET             ; 10 T
