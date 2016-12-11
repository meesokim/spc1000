# Uncomment the one corresponding with your system
 
#SYSTEM=DOS
SYSTEM=DOS

Z80_PORTS="./.Z80_ports"

DEFINES=-DFURTHER_HARDWARE -DPORT_BUFFER
#DEFINES=-DFURTHER_HARDWARE -DPORT_BUFFER -DNO_EXTERN_PORTFILE_WRITE
#DEFINES=-DFURTHER_HARDWARE -DZ80_CTC -DLOGIC_ANALYZER -DSSL_ZDS
#DEFINES=-DFURTHER_HARDWARE -DOLD_STYLE -DNEEDED

# set the appropriate install directories in your system

MAN_DIR=/usr/local/man
BIN_DIR=/usr/local/bin

#--------- You shouldn't need to change anything under this line ---------------

ZZ=Z80_PORTS="\"$(Z80_PORTS)\""
ZZZ=Z80_PORTS="\\\"$(Z80_PORTS)\\\""
HW=hardware/hard.a
ifeq (UNIX, $(SYSTEM))
 RM=rm -f
 ASM=z80-asm
 MONI=z80-mon
else
 RM=rm -f
 ASM=z80-asm.exe
 MONI=z80-mon.exe
endif

CC=	gcc
CFLAGS=-O2 -Wall -D$(ZZ) -D$(SYSTEM) $(DEFINES) -W -Wstrict-prototypes \
       -Wno-parentheses -fomit-frame-pointer -falign-functions=0
LDFLAGS=

all:
	cd hardware; $(MAKE) SYSTEM=$(SYSTEM) DEFINES="-D$(ZZZ) -D$(SYSTEM) $(DEFINES)" hard.a
	make $(ASM)
	make $(MONI)
	cd hardware; $(MAKE) SYSTEM=$(SYSTEM) DEFINES="-D$(ZZZ) -D$(SYSTEM) $(DEFINES)"

clean:
	$(RM) *.o
	$(RM) $(ASM)
	$(RM) $(MONI)
	$(RM) asm.a
	$(RM) cpu.a
	$(RM) .bus_proto
	cd hardware; make SYSTEM=$(SYSTEM) clean

install:
	chmod a+rx $(ASM) $(MONI); cp -p $(ASM) $(MONI) $(BIN_DIR)
	chmod a+r doc/man/z80-*.1; cp -p doc/man/z80-*.1 $(MAN_DIR)/man1
	chmod a+r doc/man/z80-*.3; cp -p doc/man/z80-*.3 $(MAN_DIR)/man3
	chmod a+r doc/man/z80-*.5; cp -p doc/man/z80-*.5 $(MAN_DIR)/man5

z80-mon.o: z80-mon.c z80-cpu.h asm.h console.h console_token regs.h regs_token \
           execute.h execute_token file.h memory.h interrupt.h help_layout \
           mini-display.h decode.h keyboard.h ports.h hash.h z80-global \
           hardware/includes

z80-asm.o: z80-asm.c hash.h asm.h file.h asm_token

z80-cpu.o: z80-global z80-cpu.c hardware/quartz.h z80-mon.h hardware/includes \
           hardware/system_wired hardware/bus_masters

decode.o: decode.c z80-cpu.h z80-mon.h decode-table.h execute.h execute_token \
          regs_token memory.h z80-global hardware/daisy_chain.h

instr.o: instr.c instr_token z80-cpu.h z80-global

file.o: file.c file.h

regs.o: regs.c regs_token z80-cpu.h z80-global

compile.o: compile.c asm.h execute_token regs.h regs_token

expression.o: expression.c asm.h asm_token

hash.o: hash.c asm.h expression.h

asm.o: asm.c asm_interface.h instr.h z80-cpu.h hash.h compile.h execute_token \
       expression.h regs.h regs_token asm_token z80-global

console.o: console.c console_token

decode-table.o: decode-table.c z80-cpu.h execute.h execute_token regs.h \
                regs_token z80-global

execute.o: execute.c z80-cpu.h execute_token regs.h regs_token decode.h asm.h \
           memory.h ports.h hash.h z80-global

ports.o: ports.c z80-cpu.h mini-display.h asm_interface.h keyboard.h memory.h \
         hardware/port_buffer.h z80-global

memory.o: memory.c z80-cpu.h z80-mon.h z80-global

interrupt.o: interrupt.c z80-cpu.h decode.h z80-mon.h z80-global \
             hardware/daisy_chain.h

dummy.o: dummy.c

mini-display.o: mini-display.c console.h console_token

keyboard.o: keyboard.c console.h

asm.a: z80-cpu.o asm.o hash.o compile.o regs.o instr.o interrupt.o file.o \
       expression.o mini-display.o keyboard.o
	$(RM) asm.a
	ar rcs asm.a z80-cpu.o asm.o hash.o compile.o regs.o instr.o interrupt.o \
                 expression.o mini-display.o keyboard.o file.o

cpu.a: execute.o decode-table.o decode.o memory.o ports.o
	$(RM) cpu.a
	ar rcs cpu.a execute.o decode-table.o decode.o memory.o ports.o


$(ASM): z80-asm.o dummy.o asm.a $(HW)
	gcc  -o $(ASM) z80-asm.o dummy.o asm.a $(HW)
$(MONI): z80-mon.o cpu.a console.o asm.a $(HW)
	gcc -lc -o $(MONI) z80-mon.o cpu.a console.o asm.a $(HW)
