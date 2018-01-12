#--------- You shouldn't need to change anything under this line ---------------

ifeq (UNIX, $(SYSTEM))
 RM=rm -f
 CLK=hardware_clock
 LCD=lcd_display
 MKP=make_ports
 IOP=port_io
 MON=monitor_cpu
else
 RM=rm -f
 CLK=hw_clk.exe
 LCD=lcd_dis.exe
 MKP=mk_ports.exe
 IOP=port_io.exe
 MON=monitor_cpu.exe
endif

CC=		gcc
CFLAGS=-O2 -Wall -D$(SYSTEM) $(DEFINES) -W -Wstrict-prototypes \
       -Wno-parentheses -fomit-frame-pointer -falign-functions=0
LDFLAGS=

all: $(CLK) $(LCD) $(MKP) $(IOP) $(MON) quartz.o daisy_chain.o z80-ctc.o \
     port_buffer.o analyzer.o ssl_zds.o

hard.a:	quartz.o daisy_chain.o z80-ctc.o port_buffer.o analyzer.o ssl_zds.o
	$(RM) hard.a
	ar rcs hard.a quartz.o daisy_chain.o z80-ctc.o port_buffer.o analyzer.o \
       ssl_zds.o

clean:
	$(RM) hard.a
	$(RM) *.o
	$(RM) $(CLK)
	$(RM) $(LCD)
	$(RM) $(MKP)
	$(RM) $(IOP)
	$(RM) $(MON)

lcd_display.o: lcd_display.c ../console.h ../console_token ../z80-cpu.h ../z80-global

hardware_clock.o: hardware_clock.c ../z80-cpu.h ../z80-global

make_ports.o: make_ports.c ../z80-cpu.h ../z80-global

port_io.o:	port_io.c ../z80-cpu.h ../z80-global

quartz.o:	quartz.c includes

daisy_chain.o:	daisy_chain.c ../z80-cpu.h ../z80-global

z80-ctc.o:	z80-ctc.c ../z80-global daisy_chain.h

port_buffer.o:	port_buffer.c ../z80-mon.h ../z80-cpu.h ../z80-global

analyzer.o:	analyzer.c ../z80-cpu.h ../z80-global

ssl_zds.o: ssl_zds.c ../z80-cpu.h ../z80-global zds_token

$(LCD): lcd_display.o ../dummy.o ../cpu.a ../console.o ../asm.a hard.a
	gcc -lc -o $(LCD) lcd_display.o ../dummy.o ../cpu.a ../console.o ../asm.a hard.a
$(CLK): hardware_clock.o
	gcc -lc -o $(CLK) hardware_clock.o
$(MKP): make_ports.o
	gcc -lc -o $(MKP) make_ports.o
$(IOP): port_io.o
	gcc -lc -o $(IOP) port_io.o
$(MON): monitor_cpu.o ../cpu.a ../console.o ../asm.a
	gcc -lc -o $(MON) monitor_cpu.o ../cpu.a ../console.o ../asm.a
