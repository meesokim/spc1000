#
# Makefile - VCHIQ sound + emu2149
#

CIRCLEHOME ?= ~/circle
RASPPI ?= 1

USE_VCHIQ_SOUND = 1
CFLAGS += -DUSE_VCHIQ_SOUND=$(USE_VCHIQ_SOUND)
CPPFLAGS += -DUSE_VCHIQ_SOUND=$(USE_VCHIQ_SOUND)

KERNEL_MAX_SIZE = 0x400000
DEFINE += -DKERNEL_MAX_SIZE=$(KERNEL_MAX_SIZE)

OBJS = main.o kernel.o MC6847.o Z80SPC.o spcall.o emu2149.o tap.o tape_loader.o cassette.o stub.o \
       miniz/miniz.o miniz/miniz_tdef.o miniz/miniz_tinfl.o miniz/miniz_zip.o \
       bzip2/blocksort.o bzip2/huffman.o bzip2/crctable.o bzip2/randtable.o bzip2/compress.o bzip2/decompress.o bzip2/bzlib.o

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

INCLUDE = -I$(CIRCLEHOME)/addon -Iminiz -Ibzip2 -I.

tap.o: tap.c

tap.c: sharkkiller.tap
	@echo "Generating tap.c from $<..."
	@echo 'char tap0[] = {' > $@
	@echo -n '"' >> $@
	@tail -n 1 $< | tr -d '\r\n' >> $@
	@echo '"};' >> $@

host:
	$(MAKE) -f Makefile.host

host-clean:
	$(MAKE) -f Makefile.host clean

include $(CIRCLEHOME)/Rules.mk
