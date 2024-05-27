#
# Makefile
#

#USE_VCHIQ_SOUND = 1

# CIRCLEHOME = $(CIRCLE)

OBJS	= main.o kernel.o oscillator.o

ifeq ($(strip $(USE_VCHIQ_SOUND)),1)
LIBS	= $(CIRCLEHOME)/addon/vc4/sound/libvchiqsound.a \
	  $(CIRCLEHOME)/addon/vc4/vchiq/libvchiq.a \
	  $(CIRCLEHOME)/addon/linux/liblinuxemu.a

CFLAGS += -DUSE_VCHIQ_SOUND=$(USE_VCHIQ_SOUND) -D__CIRCLE__
DEFINE += -D__CIRCLE__
endif

LIBS	+= $(CIRCLEHOME)/lib/sched/libsched.a \
	   $(CIRCLEHOME)/lib/sound/libsound.a \
	   $(CIRCLEHOME)/lib/usb/libusb.a \
	   $(CIRCLEHOME)/lib/input/libinput.a \
	   $(CIRCLEHOME)/lib/fs/libfs.a \
	   $(CIRCLEHOME)/lib/libcircle.a \
	   SDL2/libSDL2.a \
	   SDL2_image/libSDL2_image.a \
	   SDL2_mixer/libSDL2_mixer.a \
	   kernel/libkernel.a

include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)

INCLUDE += -ISDL2/include

all:
	make -C zlib
	make -C libpng
	make -C libogg
	make -C libvorbis
	make -C kernel
	make -C SDL2
	make -C SDL2_image
	make -C SDL2_mixer
	make

clean1: clean
	make -C zlib -k clean
	make -C libpng -k clean
	make -C libogg -k clean
	make -C libvorbis -k clean
	make -C SDL2 -k clean
	make -C SDL2_image -k clean
	make -C SDL2_mixer -k clean
	make -C kernel -k clean
	rm -f *.o *.d *.elf *.lst *.map *.img
