;==============================================================================
;       Startup Code for SPC
;
;           crt0.s
;                                   blog.naver.com/mayhouse
;                                    Based on  Alteration by Piroyan.    Based on Crt0.s included in SDCC.
;==============================================================================

		.module	crt0spc
		.globl	_main

	.area	_HEADER	(ABS)

	.org	0x7cdd		; 7cdd번지부터 프로그램끝주소와 시작주소가 들어간다. 6byte
					;그러므로 소스는 8006번지 이후로 들어가면된다.
					;혹은 원하는 주소로 수정하면된다.
					;0x8000이면 --code-loc 0x8006
					;0x7cdd이면 --code-loc 0x7ce3
start:
	call	gsinit		; Initialise global variables
	jp		_main		; Jump to main()


	; Ordering of segments for the linker.
	.area	_HOME
	.area	_CODE
	.area	_GSINIT
	.area	_GSFINAL

	.area	_GSINIT
gsinit::

	; Initialization code enters here. 

	.area	_GSFINAL
	ret

	.area	_DATA
	.area	_BSS
	.area	_HEAP
	