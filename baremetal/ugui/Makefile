#
# Makefile
#

CIRCLEHOME = ~/circle

DEPTH=8
USE_COLOR_INDEX=1

OBJS	= ugui.o uguicpp.o

CFLAGS	+= -DUSE_COLOR_INDEX

libugui$(RASPPI).a: $(OBJS)
	rm -f $@
	$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk
