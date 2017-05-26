CC = gcc
NAME  = spcemul


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

SRC1	  = Z80SPC.c    \
		AY8910.c    \
		MC6847.c    \
		menus.c    \
		gifsave.c \
		Debug.c
		

SRC2	= spcmain.cpp
		
OBJ1	= $(SRC1:.c=.o)
OBJ2	= $(SRC2:.cpp=.o)
OBJ= $(OBJ1) $(OBJ2)

ifeq ($(TARGETOS),win32)
DEFS = -DWIN32
DEFL = -Llib -lcomdlg32 -lmingw32 -lWinmm
endif

ifeq ($(TARGETOS),linux)
DEFS = -DLINUX
DEFL = -lrt 
endif

CFLAGS		+= $(DEFS) -I/mingw/include/SDL -I/usr/include/SDL -I/usr/local/include/SDL -Iinclude -Iinclude/SDL 

LDFLAGS		+= $(DEFL) -static -lSDLmain -lSDL_image -lSDL_ttf -lstdc++ -lSDL.dll

$(NAME): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@
%.o: %.cpp
	g++ -c $< $(CFLAGS) -o $@

all: $(NAME)

clean:
	rm -f *.o
	rm -f $(NAME)
