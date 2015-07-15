NAME  = sd720
CC = sdcc
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
		
OBJ1	= $(SRC1:.c=.rel)
OBJ2	= $(SRC2:.s=.rel)
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
LDFLAGS	= -mz80 --code-loc 0x7ce3  --data-loc 0x9000 --no-std-crt0 

$(NAME): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@.ihex
	bin2tap $@.ihex

%.rel: %.c
	$(CC) -c $< $(CFLAGS) -o $@
%.rel: %.s
	$(AS) -l -o $< 

all: $(NAME)

clean:
	rm -f *.rel
