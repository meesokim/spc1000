#define     progStart   $cb00
.org        progStart
	ld de, Message
    call $7f3
	ret
Message:
.db         "Hello world!",0
