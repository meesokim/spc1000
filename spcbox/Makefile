#
# Makefile
#

CIRCLEHOME = ../../circle

RASPPI ?= 1

STDLIB_SUPPORT=3

OBJS	= kernel.o main.o miniz0.o platform.o

LIBS	= lib$(RASPPI)/libcircle.a \
		  lib$(RASPPI)/libsdcard.a \
		  lib$(RASPPI)/libfatfs.a \
		  lib$(RASPPI)/libfs.a 

# CPPFLAGS += -fno-unwind-tables
# LDFLAGS += -fno-use-cxa-atexit -O2 -fno-threadsafe-statics -shared

include $(CIRCLEHOME)/Rules.mk



