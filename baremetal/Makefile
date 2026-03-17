#
# Makefile
#

CIRCLEHOME ?= ../circle
RASPPI ?= 1

ifeq ($(strip $(RASPPI)),1)
TARGET = kernel
else ifeq ($(strip $(RASPPI)),2)
TARGET = kernel7
else ifeq ($(strip $(RASPPI)),3)
TARGET = kernel8-32
else ifeq ($(strip $(RASPPI)),4)
TARGET = kernel7l
endif

USE_VCHIQ_SOUND=1

INCLUDE = -I$(CIRCLEHOME)/addon
TAP_FILE = apple.tap
BMP = samsung
GCC = gcc

ifeq ($(strip $(USE_VCHIQ_SOUND)),1)
LIBS    = $(CIRCLEHOME)/addon/vc4/sound/libvchiqsound.a \
          $(CIRCLEHOME)/addon/vc4/vchiq/libvchiq.a \
		  $(CIRCLEHOME)/lib/sched/libsched.a \
          $(CIRCLEHOME)/addon/linux/liblinuxemu.a

CFLAGS += -DUSE_VCHIQ_SOUND=$(USE_VCHIQ_SOUND)
CPPFLAGS += -DUSE_VCHIQ_SOUND=$(USE_VCHIQ_SOUND)
endif

OBJS = main.o kernel.o Z80SPC.o MC6847.o AY8910.o casswindow.o screen8.o mutex.o spcall.o tap.o samsung.bmp.o 

LIBS += ugui/libugui$(RASPPI).a \
	   $(CIRCLEHOME)/lib/usb/libusb.a \
	   $(CIRCLEHOME)/lib/fs/libfs.a \
	   $(CIRCLEHOME)/lib/input/libinput.a \
	   $(CIRCLEHOME)/lib/sound/libsound.a \
	   $(CIRCLEHOME)/addon/fatfs/libfatfs.a \
	   $(CIRCLEHOME)/lib/libcircle.a 

EXTRACLEAN = sound.h samsung.bmp.c tap.c *.bak *.lis *.bin *.inc spcall.rom spcall.c spcall.h

# Define the default target before including Rules.mk
all: z80asm spcall.c sound.h $(TARGET).img

include $(CIRCLEHOME)/Rules.mk

$(BMP).bmp.c: $(BMP).bmp bmp2c.py 
	python3 ./bmp2c.py $(BMP).bmp

ugui/libugui$(RASPPI).a:
	$(MAKE) -C ugui CIRCLEHOME=../../circle RASPPI=$(RASPPI)

z80asm:
	$(MAKE) -C ../z80/z80asm
	cp ../z80/z80asm/z80asm bin/
	
bmp2bin:
	$(GCC) -o $@ $@.c

sound.h: sound.h.gz
	gunzip -c sound.h.gz > sound.h

tap.c: $(TAP_FILE)
	./tap2c $(TAP_FILE) > tap.c

# Rule for assembling z80 code
%.bin: %.asm
	bin/z80asm -fb -l -sa $< 

# spcall.c depends on the assembled z80 binaries
spcall.c: spc-1000_all.bin spc-1000_ipl.bin bin2c.py
	cat spc-1000_all.bin spc-1000_ipl.bin > spcall.rom
	python3 bin2c.py spcall.rom ROM
