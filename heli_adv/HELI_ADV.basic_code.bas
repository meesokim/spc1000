10 REM
20 REM HELI ADVENTURE
30 REM      for SPC-1000
40 REM         by C.S.JANG
50 REM
100 CLEAR &HA000
110 IF PEEK (&HA000) = &HCD GOTO 130
120 GOSUB 280
130 CALL &HAFD0
140 SCREEN 0, 0, 5
150 CLS
160 LINE (0, 122) - (255, 191), PSET, 1, BF
170 CALL &HB0B3
180 CALL &HB0C3
190 LOCATE 4, 2
200 PRINT "PUSH S KEY TO GAME START"
210 CALL &HB0B3
220 CALL &HB0C3 
230 I$ = INKEY$
240 IF I$="s" OR I$="S" THEN GOTO 260
250 GOTO 230
260 CALL &HA000
270 GOTO 190
280 CLS 
290 PRINT "MACHINE LANGUAGE LOADING..."
300 PRINT "LOAD"
310 LOAD
320 RETURN