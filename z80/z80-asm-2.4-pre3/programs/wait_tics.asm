; in register A is the number of tics this routine will last
; A must be either 44 or 45 or at least MAG+14=48 !!
; on return the following registers are undefined: AF, HL
; if wait_tics0 is called, H must already contain the value tab
; and A must be already decreased by 20. Then we handle all timings
; 30,31,34,35, ... 255 which corresponds to A = 10,11,14,15, ...

wait_tics1:
MAG    EQU  34
       SUB MAG           ; 7
       LD H,tab/256      ; 7
wait_tics0:
       RRCA              ; 4
       RRCA              ; 4
       CPL               ; 4
       LD L,A            ; 4
       JP (HL)           ; 4
;
; in register DE (high) and BC (low) is the number of tics the routine will last
; if DE is zero then BC must be at least 141+111=252 !!
       PUSH AF            ; 11
       PUSH HL            ; 11
       LD H,B             ; 4 
       LD L,C             ; 4 
       LD BC,-141         ; 10   ; ff73H = 65536 - 141
       ADD HL,BC          ; 11
       JP C,over0         ; 10  
       DEC DE             ; 6
       NOP                ; 4
over1: LD B,H             ; 4 
       LD C,L             ; 4 
bb0:   LD A,D          ; 4
       OR E            ; 4
       JR NZ,lloo      ; 7/12
       CALL wait_tics2    ; 17
       POP HL             ; 10
       POP AF             ; 10
       RET                ; 10
over0: JP over1           ; 10
lloo:  DEC DE          ; 6
       PUSH BC         ; 11
       LD BC,65452     ; 10    65452=65536-(4+4+12+6+11+10+17+10+10)
       CALL wait_tics2 ; 17
       POP BC          ; 10
       JP bb0          ; 10
;
; in register BC is the number of tics this routine will last
; if B equal 0 then C must be at least MAGIC+40=111 !!
; on return the following registers are undefined: AF, BC, HL
wait_tics2:
       INC B             ; 4
       DJNZ tozero       ; 8/13
       LD A,C            ; 4
; borrow 16+(MAGIC+24) tics from A
       SUB 111           ; 7   ; (4+8+4)+MAGIC+(7+7+10)
       CP LIMIT          ; 7
       JP C,rest         ; 10
back:  SUB LIMIT         ; 7
cont:
; now we have MAGIC+17+C to spend
; waste as much as possible tics but at most A many
LIMIT  EQU  24           ;  24 = 7+10+7
try:
       CP LIMIT          ; 7
       JP NC,back        ; 10
;      A is at most LIMIT-1
; end waste
;     now we need at least MAGIC=57+14 tics and A counts the superflious tics
MAGIC EQU   71
rest:
; now we have MAGIC+A tics remaining which are at most MAGIC+LIMIT-1
       LD HL,table       ; 10
       ADD A,A           ; 4
       LD C,A            ; 4    ; B is 0
       ADD HL,BC         ; 11   ; Carry not set
       LD A,(HL)         ; 7
       INC HL            ; 6
       LD H,(HL)         ; 7
       LD L,A            ; 4
       JP (HL)           ; 4
loop:
; waste 256-13 = 243 tics
       LD A,14           ; 7
rep:   DEC A             ; 4
       JR NZ,rep         ; 12   ; (A-1)*16+7+11=A*16+2 = 226
       RET NZ            ; 5
       INC HL            ; 6
       DEC HL            ; 6
; end waste
tozero:
       DJNZ  loop        ; 8/13
; now we have -25+256+C tics to spend
; waste -25+(256-MAGIC-31) = 129 tics
       LD B,9            ; 7
wast:  DJNZ wast         ; 13 ; 13*B+2
       RET NZ            ; 5
       RET NZ            ; 5
; end waste
; now we have MAGIC+31+C to spend
       LD A,C            ; 4
       JP cont           ; 10
table: DEFW  t14
       DEFW  t15
       DEFW  t16
       DEFW  t17
       DEFW  t18
       DEFW  t19
       DEFW  t20
       DEFW  t21
       DEFW  t22
       DEFW  t23
       DEFW  t24
       DEFW  t25
       DEFW  t26
       DEFW  t27
       DEFW  t28
       DEFW  t29
       DEFW  t30
       DEFW  t31
       DEFW  t32
       DEFW  t33
       DEFW  t34
       DEFW  t35
       DEFW  t36
       DEFW  t37
       ALIGN  8
tab:                     ; tab must be aligned to 256-boundary !!
       DEFS 41
t91:   DEFS 14
t35:   NOP
t31:   NOP
t27:   NOP
t23:   NOP
t19:   NOP
t15:   NOP  
t11:   RET NC            ; Carry not set
       DEFS 43
t90:   DEFS 14
t34:   NOP 
t30:   NOP 
t26:   NOP 
t22:   NOP 
t18:   NOP 
t14:   NOP
t10:   RET
       DEFS 42
t93:   DEFS 14
t37:   NOP
t33:   NOP
t29:   NOP
t25:   NOP
t21:   NOP
t17:   LD L,0
       RET
       DEFS 42
t92:   DEFS 14
t36:   NOP
t32:   NOP
t28:   NOP
t24:   NOP
t20:   NOP   
t16:   INC HL
       RET
       DEFS 3 ; now three bytes reserve upto next 256 boundary:)
;
       NOP        ; 4
       RET NC     ; 5 Carry set
       INC HL     ; 6           ; 6
       JR NC,+0   ; 7 Carry set ; 3.5
       NOP       
       NOP        ; 8           ; 4
       NOP
       RET NC     ; 9 Carry set ; 4.5
       JP Z,@     ; 10          ; 3.333
       JR NC,@+2  ;   Carry set
       NOP        ; 11          ; 3.666
       DEC HL
       INC HL     ; 12  ; 2     ; 6
       NOP
       NOP
       RET NC     ; 13 Carry set; 4.333
       NOP
       RET NC     ; Carry set
       RET NC     ; 14 Carry set; 4.666
       NOP
       NOP
       JR NC,+0   ; 15 Carry set; 3.75
       NOP
       DEC HL
       INC HL     ; 16  ; 3     ; 5.333
       RET NC     ; Carry set
       DEC HL
       INC HL     ; 17  ; 3     ; 5.666
       NOP
       NOP
       RET NC     ; Carry set
       RET NC     ; 18 Carry set; 4.5
       JR Z,+0    ;
       JR NZ,+0   ; 19  ; 4     ; 4.75
       NOP         
       NOP         
       JR C,+0    ; 20 Carry set; 5
       PUSH AF   
       POP AF     ; 21  ; 2     ; 10.5
       RET NC     ; Carry set
       RET NC     ; Carry set
       DEC HL
       INC HL     ; 22  ; 4     ; 5.5
       NOP
       JR Z,+0    ;
       JR NZ,+0   ; 23  ; 5     ; 4.6
       DEC HL
       INC HL 
       DEC HL
       INC HL     ; 24  ; 4     ; 6
       NOP
       PUSH AF   
       POP AF     ; 25  ; 3     ; 8.333
       RET NC     ; Carry set
       PUSH AF   
       POP AF     ; 26  ; 3     ; 8.666
       RET NC     ; Carry set
       CALL +0
       RET        ; 27  ; 4     ; 6.75
       CALL +0
       RET C      ; 28 Carry set; 7
       NOP
       NOP
       PUSH AF   
       POP AF     ; 29  ; 4     ; 7.25
       NOP
       RET NC     ; Carry set 
       PUSH AF   
       POP AF     ; 30  ; 4     ; 7.5
       JR NC,+0   ; Carry set
       CALL +0
       NOP
       RET        ; 31  ; 5     ; 6.2
       CALL +0
       NOP
       RET C      ; 32 Carry set; 6.4
       JR C,+0    ; Carry set
       PUSH AF   
       POP AF     ; 33  ; 4     ; 8.25

       CALL xxxx  ; all >= 34
xxxx:
            RET   ; 27 + xxxx
