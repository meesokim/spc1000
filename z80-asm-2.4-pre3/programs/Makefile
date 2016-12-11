RM=rm -f
CC=	gcc
CFLAGS=-O2 -Wall -W -Wstrict-prototypes -Wno-parentheses -fomit-frame-pointer -falign-functions=0
LDFLAGS=

all: make_header opcodes

clean:
		$(RM) *.o
		$(RM) make_header opcodes

make_header.o:	make_header.c

opcodes.o:	opcodes.c

make_header: make_header.o
		gcc -o make_header make_header.o
opcodes: opcodes.o
		gcc -o opcodes opcodes.o

