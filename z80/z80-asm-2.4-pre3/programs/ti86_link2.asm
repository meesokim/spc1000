;======================================================
; ReceiveByte                 [Assembly Coder's Zenith]
;  Originally by Pascal Bouron and Jimmy Mardell
;  Modified by Joe Clussman, 1 July 2002
;
; in:  D = timeout period (16? 200? it depends.)
; out: A = received byte    carry set = error
;      OP1 = time for each bit
;      HL = timer where timeout occurred (only if it occurred)
;destroyed: AF, BC, E, HL
;
;
; Notes:
;   Outputting is inverted.  To send 0V, write a 1
;    Inputting wire status is 0 = GND, 1 = +5V
;     -only bits 0 and 1 are significant in inputting.
;
;   0V overtakes +5V, due to the open-collector design.
;    There is a Weak pull-up to +5V on the Red and White wires.
;
;   The LSB is sent first, the MSB last-- the eighth total bit.
;    There is no Parity, Start, or Stop bits.
;======================================================
_OP1    equ         0C089h
;
ReceiveByte:
;    ld hl,0
;    ld (_OP1),hl        ;temp storage = 0
    ld hl,_OP1
    ld b,8              ; counter
    ld c,0              ; byte receive
    ld e,1              ; for the OR
    ld a,$C0            ;set W = +5V, R = +5V
    out     (7),a
    
    ld (hl),c           ;load first byte of OP1 with a zero...
rb_w_Start:
    in      a,(7)       ;read link status
    and 3
    cp  3
    jr NZ,rb_get_bit    ;IF both W and R are NOT at both at +5V, get the bit
    call rbChkTimeOut   ;check for timeout
    jr rb_w_Start       ;loop back up to check for data again...
rb_get_bit:
    cp %00000010        ;check bit 1  (the White wire)
    jr Z,rb_receive_zero    ;if W = +5V (bit 1 = 1), and R = 0 (bit 0 = 0)
                            ; then receive a zero.
    ;-at this point, W = 0V (bit 1 = 0) and R = undef. (assum. 0)
    ;  SO... this bit we have here is a ONE.
    ld a,c              ;
    or e                ;set bit of C
    ld c,a              ;
    ld a,$D4            ;W = +5V, R = 0V   (do this to make both W and R at 0V)
    out (7),a
    jr rb_waitStop      ;skip to below
rb_receive_zero:
    ld a,$E8
    out (7),a           ;W = 0V, R = +5V  (acknowledge by making W and R at 0V)
                        ;don't have to clear bit of C since a zero is already there
rb_waitStop:
    call rbChkTimeOut   ;check for timeout again
    in a,(7)
    and 3
    jr z,rb_waitStop    ;keep looping until W and R stop BOTH being 0V
    ld a,$C0            ;W = +5V and R = +5V (idle state)
    out (7),a
    rl e                ;move over the 1 in E to a higher Sig. Bit.
    
    inc HL
    ld (HL),0
    djnz rb_w_Start     ;B = B - 1.  jump if B <> 0
    xor a               ;clear carry flag no error
    ld a,c
    ret

rbChkTimeOut:
    ld a,(HL)           ;check current timer of OP1 for limit in reg D
    cp d
    inc a
    ld (HL),a
    ret NZ              ;flags from cp still valid, return if A != D
rbTimeOut
    pop BC              ;destroy call address
    ld a,$C0            ;idle state
    out (7),a
    xor A               ;clear A
    scf                 ;bleh, set carry flag for error
    ret



;======================================================
; SendByte                    [Assembly Coder's Zenith]
;  Originally by Pascal Bouron and Jimmy Mardell
;  Modified by Joe Clussman, 1 July 2002
;
; in: A = byte to send
; out: carry set = error
; destroyed: all registers, OP1
;======================================================
SendByte:
    ld hl,0
    ld (_OP1),hl
    ld b,8
    ld c,a                  ;byte to send
    ld a,$C0                ;W = +5V, R = +5V  (quiet state)
    out (7),a
sbwait4_idle
    in a,(7)                ;read link status
    and 3
    cp  3                   ;see if both W and R are +5V (in quiet state)
    jr Z,sbcalc_bit         ;if they are, jump to where we send
    call sbChkTimeOut       ;ELSE check for Exit key
    jr sbwait4_idle         ;and loop back
sbcalc_bit:
    ld      a,c             ;A = byte to send
    and     1               ;want only bit one

    jr      z,sbsend_zero   ;IF Bit zero is zero, THEN send a zero, ELSE send one (!)
sbsend_one:
    ld      a,$E8           ;W = 0V, R = +5V
    out     (7),A
    jr      sbwait4_ack
sbsend_zero:
    ld      a,$D4           ;W = +5V, R = 0V
    out     (7),A
sbwait4_ack                 ;sbwait_setport:
    call sbChkTimeOut       ;check for Exit key again
    in a,(7)                ;check for Acknowledge
    and 3
    jr NZ,sbwait4_ack       ;Loop back if W and R are not both at 0V
    
    ld a,$C0                ;W = +5V, R = +5V
    out (7),A
    
    srl c                   ;shift over for next bit
    djnz sbwait4_idle       ;B = B - 1, IF B <> 0, send another bit
    xor a                   ;clear carry
    ret
    
sbChkTimeOut:
    ld a,%00111111          ;key mask for row 6 (More Exit 2nd F1-F5)
    out (1),a               ;why are we checking keypad?!
    nop
    nop
    in a,(1)                ;read key status
    bit 6,a                 ;check for EXIT key
    ret NZ                  ;if Exit isn't pressed, RET

    pop hl                  ;<------Just added all this!
    ld a,$C0                ;W = +5V, R = +5V  (quiet state)
    out (7),a
    scf                     ;error!!!

    ret
    end
