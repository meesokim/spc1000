#
# Makefile
#

CIRCLEHOME = ~/circle

USE_VCHIQ_SOUND=1

OBJS	= main.o kernel.o spc1000.o mc6847.o emu2149.o \
		  miniz/miniz_tdef.o miniz/miniz_tinfl.o miniz/miniz.o \
		  bzip2/crctable.o 

ifeq ($(strip $(USE_VCHIQ_SOUND)),1)
LIBS	= $(CIRCLEHOME)/addon/vc4/sound/libvchiqsound.a \
	  $(CIRCLEHOME)/addon/vc4/vchiq/libvchiq.a \
	  $(CIRCLEHOME)/addon/linux/liblinuxemu.a

CFLAGS += -DUSE_VCHIQ_SOUND=$(USE_VCHIQ_SOUND)
endif

CFLAGS += -Iminiz -I.

LIBS	+= $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/usb/gadget/libusbgadget.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a \
	  $(CIRCLEHOME)/lib/sound/libsound.a \
	  $(CIRCLEHOME)/lib/libcircle.a

include Rules.mk

-include $(DEPS)
