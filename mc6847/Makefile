#
# Makefile
#

CIRCLEHOME = ~/circle

OBJS	= MC6847Screen.o

MC6847Screen.a: $(OBJS)
	@echo "  AR    $@"
	@rm -f $@
	@$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)
