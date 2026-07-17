#
# Makefile - step 5: add USB keyboard
#

CIRCLEHOME ?= ~/circle
RASPPI ?= 1

OBJS = main.o kernel.o MC6847.o Z80SPC.o spcall.o

LIBS = $(CIRCLEHOME)/lib/usb/libusb.a \
       $(CIRCLEHOME)/lib/sched/libsched.a \
       $(CIRCLEHOME)/lib/input/libinput.a \
       $(CIRCLEHOME)/lib/fs/libfs.a \
       $(CIRCLEHOME)/lib/libcircle.a

INCLUDE = -I.

include $(CIRCLEHOME)/Rules.mk
