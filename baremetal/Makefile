#
# Makefile
#
#

#ifeq ($(strip $(DEPTH)), 8)
#LIBCIRCLE = libcircle8
#else
#LIBCIRCLE = libcircle
#endif

CIRCLEHOME = ~/cc
INCLUDE = -I$(CIRCLEHOME)/addon
TAP_FILE = apple.tap
BMP = samsung
GCC = gcc

#OBJS	= main.o kernel.o
OBJS = main.o kernel.o Z80SPC.o MC6847.o AY8910.o casswindow.o screen8.o mutex.o pwmsound.o spcall.o tap.o samsung.bmp.o 

LIBS =ugui/libugui$(RASPPI).a \
	  $(CIRCLEHOME)/lib/usb/libusb$(RASPPI).a \
	  $(CIRCLEHOME)/lib/input/libinput$(RASPPI).a \
	  $(CIRCLEHOME)/lib/fs/fat/libfatfs$(RASPPI).a \
	  $(CIRCLEHOME)/lib/fs/libfs$(RASPPI).a \
	  $(CIRCLEHOME)/lib/libcircle$(RASPPI).a 

EXTRACLEAN = sound.h samsung.bmp.c tap.c *.bak *.lis *.bin *.inc


include $(CIRCLEHOME)/Rules.mk

$(BMP).bmp.o: bmp2bin 
	./bmp2bin $(BMP).bmp
	$(CC) $(CFLAGS) -c -o $(BMP).bmp.o $(BMP).bmp.c

ugui/libugui$(RASPPI).a:
	$(MAKE) -C ugui
	
$(CIRCLEHOME)/lib/usb/libusb$(RASPPI).a:
	$(MAKE) -C $(CIRCLEHOME)/lib/usb clean
	$(MAKE) -C $(CIRCLEHOME)/lib/usb

$(CIRCLEHOME)/lib/input/libinput$(RASPPI).a:
	$(MAKE) -C $(CIRCLEHOME)/lib/input clean
	$(MAKE) -C $(CIRCLEHOME)/lib/input clean
	
$(CIRCLEHOME)/lib/fs/libfs$(RASPPI).a:
	$(MAKE) -C $(CIRCLEHOME)/lib/fs	clean	
	$(MAKE) -C $(CIRCLEHOME)/lib/fs		

$(CIRCLEHOME)/lib/fs/fat/libfatfs$(RASPPI).a:
	$(MAKE) -C $(CIRCLEHOME)/lib/fs/fat clean
	$(MAKE) -C $(CIRCLEHOME)/lib/fs/fat

$(CIRCLEHOME)/lib/libcircle$(RASPPI).a:
	$(MAKE) -C $(CIRCLEHOME)/lib clean
	$(MAKE) -C $(CIRCLEHOME)/lib
	
z80asm:
	$(MAKE) -C ../z80/z80asm
	cp ../z80/z80asm/z80asm .
	
bmp2bin:
	$(GCC) -o $@ $@.c

sound.h: sound.h.gz
	gunzip -c sound.h.gz > sound.h

tap.o: $(TAP_FILE)
	./tap2c $(TAP_FILE) > tap.c
	$(CC) $(CFLAGS) -c -o $@ tap.c

%.bin: %.asm
	z80asm -fb -l -sa $< 

spcall.c: spc-1000_all.bin spc-1000_ipl.bin
	cat spc-1000_all.bin spc-1000_ipl.bin > spcall.rom
	bin2c.py spcall.rom ROM

all: z80asm $(OBJS) bmp2bin sound.h %.bin