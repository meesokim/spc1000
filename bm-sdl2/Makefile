#
# Makefile
#

#ENABLE_RECORDER = 1

CIRCLEHOME = ../..

OBJS	= main.o kernel.o

ifeq ($(strip $(ENABLE_RECORDER)),1)
OBJS	+= soundrecorder.o queue.o

LIBS	= $(CIRCLEHOME)/addon/fatfs/libfatfs.a \
	  $(CIRCLEHOME)/addon/SDCard/libsdcard.a

CFLAGS += -DENABLE_RECORDER=$(ENABLE_RECORDER)
endif

LIBS	+= $(CIRCLEHOME)/lib/sched/libsched.a \
	   $(CIRCLEHOME)/lib/sound/libsound.a \
	   $(CIRCLEHOME)/lib/usb/libusb.a \
	   $(CIRCLEHOME)/lib/input/libinput.a \
	   $(CIRCLEHOME)/lib/fs/libfs.a \
	   $(CIRCLEHOME)/lib/libcircle.a

include ../Rules.mk
include SDL2/pkgconfig.mk
include SDL2_image/pkgconfig.mk
include SDL2_mixer/pkgconfig.mk
-include $(DEPS)


all:
	make -C zlib
	make -C libpng
	make -C libogg
	make -C libvorbis
	make -C SDL2
	make -C SDL2_image
	make -C SDL2_mixer

clean:
	make -C zlib -k clean
	make -C libpng -k clean
	make -C libogg -k clean
	make -C libvorbis -k clean
	make -C SDL2 -k clean
	make -C SDL2_image -k clean
	make -C SDL2_mixer -k clean
	rm -f *.o *.d *.elf *.lst *.map
