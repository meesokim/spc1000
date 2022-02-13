#
# Makefile
#

CIRCLEHOME = ~/circle

OBJS	= main.o kernel.o screen8.o tms9918.o

LIBS	= $(CIRCLEHOME)/lib/libcircle$(RASPPI).a \
		  $(CIRCLEHOME)/addon/SDCard/libsdcard$(RASPPI).a \
		  $(CIRCLEHOME)/addon/fatfs/libfatfs$(RASPPI).a \
		  $(CIRCLEHOME)/lib/fs/libfs$(RASPPI).a 

include $(CIRCLEHOME)/Rules.mk



