#
# Makefile - VCHIQ sound + emu2149
#

CIRCLEHOME ?= ~/circle
RASPPI ?= 1

USE_VCHIQ_SOUND = 1
CFLAGS += -DUSE_VCHIQ_SOUND=$(USE_VCHIQ_SOUND)
CPPFLAGS += -DUSE_VCHIQ_SOUND=$(USE_VCHIQ_SOUND)

OBJS = main.o kernel.o MC6847.o Z80SPC.o spcall.o emu2149.o tap.o

LIBS = $(CIRCLEHOME)/addon/vc4/sound/libvchiqsound.a \
       $(CIRCLEHOME)/addon/vc4/vchiq/libvchiq.a \
       $(CIRCLEHOME)/addon/linux/liblinuxemu.a \
       $(CIRCLEHOME)/lib/sched/libsched.a \
       $(CIRCLEHOME)/lib/sound/libsound.a \
       $(CIRCLEHOME)/lib/usb/libusb.a \
       $(CIRCLEHOME)/lib/input/libinput.a \
       $(CIRCLEHOME)/lib/fs/libfs.a \
       $(CIRCLEHOME)/addon/SDCard/libsdcard.a \
       $(CIRCLEHOME)/addon/fatfs/libfatfs.a \
       $(CIRCLEHOME)/lib/libcircle.a

INCLUDE = -I$(CIRCLEHOME)/addon -I.

include $(CIRCLEHOME)/Rules.mk
