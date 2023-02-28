NAME  = rpmain
BOOT  = bootpi
CC1 = sdcc
CC2 = gcc
AS = sdasz80

ifndef TARGETOS

ifeq ($(OS),Windows_NT)
TARGETOS = win32
else

ifneq ($(CROSS_BUILD),1)

ifneq ($(OS2_SHELL),)
TARGETOS = os2
else

UNAME = $(shell uname -a)

ifeq ($(firstword $(filter Linux,$(UNAME))),Linux)
TARGETOS = linux
endif
ifeq ($(firstword $(filter Solaris,$(UNAME))),Solaris)
TARGETOS = solaris
endif
ifeq ($(firstword $(filter FreeBSD,$(UNAME))),FreeBSD)
TARGETOS = freebsd
endif
ifeq ($(firstword $(filter GNU/kFreeBSD,$(UNAME))),GNU/kFreeBSD)
TARGETOS = freebsd
endif
ifeq ($(firstword $(filter NetBSD,$(UNAME))),NetBSD)
TARGETOS = netbsd
endif
ifeq ($(firstword $(filter OpenBSD,$(UNAME))),OpenBSD)
TARGETOS = openbsd
endif
ifeq ($(firstword $(filter Darwin,$(UNAME))),Darwin)
TARGETOS = macosx
endif
ifeq ($(firstword $(filter Haiku,$(UNAME))),Haiku)
TARGETOS = haiku
endif

endif

endif

endif

endif

ifndef TARGETOS
$(error Unable to detect TARGETOS from uname -a: $(UNAME))
endif

SRC1  = main.c iocs.c  		
SRC2  = sd725.s crt0spc.s 
SRC3  = bootpi.s
SRC4  = makesize.c
SRC5  = load.s
		
OBJ1	= $(SRC1:.c=.rel)
OBJ2	= $(SRC2:.s=.rel)
OBJ3 	= $(SRC3:.s=.rel)
OBJ4	= $(SRC4:.c=.o)
OBJ5	= $(SRC5:.s=.rel)
OBJ= $(OBJ1) $(OBJ2)

ifeq ($(TARGETOS),win32)
DEFS = -DWIN32
DEFL = -Llib -lcomdlg32 -lmingw32 -lWinmm
endif

ifeq ($(TARGETOS),linux)
DEFS = -DLINUX
DEFL = -lrt 
endif

CFLAGS	= -mz80 
CFLAGS3 = -I.
LDFLAGS	= -mz80 --code-loc 0xcc00 --stack-auto --data-loc 0xe400 --no-std-crt0 --code-size 0x1400 
LDFLAGS3 = 
all: load makesize $(NAME) $(BOOT) spc1000 

load: $(OBJ5)
	$(CC1) $^ $(LDFLAGS1) -o $@.ihex
	hex2bin $@.ihex

$(NAME): $(OBJ)
	$(CC1) $^ $(LDFLAGS) -o $@.ihex
	hex2bin $@.ihex
	./makesize $@.bin > $@-.bin
	mv $@-.bin $@.bin
$(BOOT): $(OBJ3)	
	$(CC1) $^ $(LDFLAGS1) -o $@.ihex
	hex2bin $@.ihex
	./makesize $@.bin 256 > boot.bin
spc1000: $(NAME) $(BOOT)
	cat boot.bin $(NAME).bin load.bin > $@.bin
	yazd --addr:0xcb00 -lst spc1000.bin > spc1000.lst
	$(eval V := $(shell ps | grep mame | grep -o -E '[0-9]+' | head -1 | sed -e 's/^0\+//'))
# if [ "$V" \> "" ]; then kill -9 $V; sleep 1; fi
# ./putspc1000 system.d88 $@.bin
makesize: $(OBJ4)
	$(CC2) $(LDFLAGS3) $^ -o $@

%.o: %.c
	$(CC2) -c $< $(CFLAGS3) -o $@
%.rel: %.c
	$(CC1) -c $< $(CFLAGS) -o $@
%.rel: %.s
	$(AS) -l -o $< 

clean:
	rm -f *.rel *.lst *.map *.sym *.noi *.ihex *.lk *.mem *.ihx *.o 
