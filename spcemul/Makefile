CC = gcc
CXX=g++
NAME  = spcemul

UNAME = $(shell uname -a)
ifndef TARGETOS

ifeq ($(firstword $(filter Linux,$(UNAME))),Linux)
TARGETOS = linux
endif

ifeq ($(OS),Windows_NT)
ifeq ($(firstword $(filter MINGW,$(UNAME))),MINGW64)
TARGETOS = win64
else
TARGETOS = win32
endif

ifneq ($(CROSS_BUILD),1)

ifneq ($(OS2_SHELL),)
TARGETOS = os2
else

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
		Debug.c \
		tms9918.c
SRC2	= spcmain.cpp
		
OBJ1	= $(SRC1:.c=.o)
OBJ2	= $(SRC2:.cpp=.o)
OBJ= $(OBJ1) $(OBJ2)

ifeq ($(TARGETOS),win32)
DEFS = -DWIN32
PRE=i686-w64-mingw32
DEFL = -L/mingw32/lib -Llib -lcomdlg32 -lmingw32 -lwinmm 
# CC=$(PRE)-gcc.exe
# CXX=$(PRE)-g++
endif

ifeq ($(TARGETOS),win64)
DEFS = -DWIN32
DEFL =  -L/mingw64/lib -Llib64 -lcomdlg32 -lwinmm -mconsole 
endif

ifeq ($(TARGETOS),linux)
DEFS = -DLINUX
DEFL = -lrt -g  -L/usr/lib/x86_64-linux-gnu/
endif

ifeq ($(TARGETOS),mingw)
DEFS = -DWIN32 -D_WIN32
DEFL = -Llib64 -lcomdlg32 -lmingw32 -lwinmm
PRE=x86_64-w64-mingw32
INCLUDE=-I/usr/share/mingw-w64/include 
CFLAGS = $(INCLUDE)
CC=$(PRE)-gcc
CXX=$(PRE)-g++
endif

CFLAGS		+= $(DEFS) -I/mingw/include/SDL -I/usr/include/SDL -I/usr/local/include/SDL -Iinclude -Iinclude/SDL 

LDFLAGS		+= $(DEFL) -lSDLmain -lm -lSDL -lSDL_image -lfreetype -lSDL_ttf -g -ggdb

$(NAME): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@ 

%.o: %.c
	$(CC)  $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX)  $(CFLAGS) -c $< -o $@

all: $(NAME)

clean:
	rm -f *.o
	rm -f $(NAME)
