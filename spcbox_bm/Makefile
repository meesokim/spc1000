RASPPI ?=1
KERNEL ?=.
include common.mk

all: $(TARGET).img 

CFLAGS += -ffunction-sections -Iminiz -Ifatfs -DRASPPI=$(RASPPI)  
SOURCES = $(shell find . -maxdepth 2 -not -path "./sjasmsrc42c/*" -not -path "./include/*" -type f \( -iname "*.c" -o -iname "*.cpp" -o -iname "*.cc" \) -print) 
ROMS    = 
ROMOBJS = $(shell echo $(ROMS) | sed -r 's/\.rom|\.mx2|\.bin/\.o/g') 
OBJS	= $(shell echo $(SOURCES) | sed -r 's/\.s|\.c|\.cpp|\.cc/\.o/g') $(ROMOBJS) 
STARTUP = start1.o
LIBS_DEP = 
LDFLAGS += -dead_strip -fno-use-cxa-atexit -O2 -fno-threadsafe-statics -lc -lnosys
-include $(DEPDIR)/*.Po

%.o: %.bin 
	@echo "  BIN   $<"
	@$(LD) -r -b binary -o $@ $<

%.o: %.rom 
	@echo "  ROM   $<"
	@$(LD) -r -b binary -o $@ $<

%.o: %.mx2
	@echo "  ROM   $<"
	@$(LD) -r -b binary -o $@ $<	

$(TARGET).img: $(OBJS) $(STARTUP) Makefile rpi$(RASPPI).ld $(LIBS_DEP)
	@echo "  LINK  $@"
	@$(CXX) $(LDFLAGS) $(OBJS) $(LIBS)
	@echo "  DUMP  $@"
	@$(OBJDUMP) -d $(TARGET).elf | $(TOOLCHAIN)c++filt > $(TARGET).lst 
	@echo "  IMAGE $@"
	@$(OBJCOPY) $(TARGET).elf -O binary $(TARGET).img
	@rm $(TARGET).elf

c: $(TARGET).img
	@c $(TARGET).img

clean:
	@echo "  CLEAN"
	@rm -f $(OBJS) *.bak kernel.elf kernel.img kernel.map kernel.lst *.o
	@rm -rf $(DEPDIR)
