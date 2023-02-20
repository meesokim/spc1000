#
# Makefile
#

CIRCLEHOME = ../../circle

RASPPI ?= 1

OBJS	= main.o spc1000.o

LIBS	= lib$(RASPPI)/libcircle.a \
		  lib$(RASPPI)/libsdcard.a \
		  lib$(RASPPI)/libfatfs.a \
		  lib$(RASPPI)/libfs.a 
# *.c: *.bin
# 	./bin2c.py $< $< char

include $(CIRCLEHOME)/Rules.mk



