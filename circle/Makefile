#
# Makefile
#

CIRCLEHOME = ~/circle

USE_VCHIQ_SOUND=1
STDLIB_SUPPORT=3

OBJS	= main.o kernel.o spc1000.o cpu.o spcall.o mc6847.o emu2149.o cassette.o ugui.o stub.o oscillator.o \
		  miniz/miniz_tdef.o miniz/miniz_tinfl.o miniz/miniz.o miniz/miniz_zip.o \
		  bzip2/crctable.o bzip2/decompress.o bzip2/huffman.o bzip2/randtable.o bzip2/bzlib.o bzip2/compress.o bzip2/blocksort.o

ifeq ($(strip $(USE_VCHIQ_SOUND)),1)
LIBS	= $(CIRCLEHOME)/addon/vc4/sound/libvchiqsound.a \
	  $(CIRCLEHOME)/addon/vc4/vchiq/libvchiq.a \
	  $(CIRCLEHOME)/addon/linux/liblinuxemu.a

CFLAGS += -DUSE_VCHIQ_SOUND=$(USE_VCHIQ_SOUND)
endif

CFLAGS += -Wno-unused-but-set-variable -Wno-unused-variable -Iminiz -Ibzip2 -I. -Wno-unused-but-set-variable -D_XOPEN_SOURCE=700
CPPFLAGS += -Wno-unused-but-set-variable -D_XOPEN_SOURCE=700
EXTRALIBS += "$(shell $(CPP) $(ARCH) -print-file-name=libc.a)" --gc-sections
LIBS	+= $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/sched/libsched.a \
	  $(CIRCLEHOME)/addon/SDCard/libsdcard.a \
	  $(CIRCLEHOME)/addon/fatfs/libfatfs.a \
	  $(CIRCLEHOME)/lib/usb/gadget/libusbgadget.a \
	  $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a \
	  $(CIRCLEHOME)/lib/sound/libsound.a \
	  lib/libcircle.a 

include Rules.mk

-include $(DEPS)
