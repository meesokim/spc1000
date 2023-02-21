
TOOLCHAIN := arm-none-eabi-
 
CC := $(TOOLCHAIN)gcc
CXX := $(TOOLCHAIN)g++
LD := $(TOOLCHAIN)ld
AS := $(TOOLCHAIN)as
AR := $(TOOLCHAIN)ar
OBJCOPY := $(TOOLCHAIN)objcopy
OBJDUMP := $(TOOLCHAIN)objdump

DEPDIR := .deps
USPI := 0

USER_ACTIONS:=\
	all copy mv

.PHONEY:$(USER_ACTIONS)
default: all

# set this to "softfp" if you want to link specific libraries
FLOAT_ABI ?= soft

RASPPI ?= 1
ifeq ($(strip $(RASPPI)),1)
TARGET=kernel
ASFLAGS = --warn -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=$(FLOAT_ABI) 
CFLAGS += -fpermissive -Wall -O3 -ffreestanding -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=$(FLOAT_ABI) -fsigned-char -I$(KERNEL) -DRASPPI=$(RASPPI) -w
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-unwind-tables -fno-rtti
LDFLAGS = -T $(KERNEL)/rpi$(RASPPI).ld -nostartfiles --specs=nosys.specs -fno-exceptions -fno-unwind-tables -fno-rtti -Wl,-Map=$(TARGET).map -o $(TARGET).elf
else ifeq ($(strip $(RASPPI)),2)
TARGET=kernel7
ASFLAGS = --warn -march=armv7-a -marm -mfpu=neon-vfpv4 -mfloat-abi=$(FLOAT_ABI) 
CFLAGS += -fpermissive -Wall -O2 -ffreestanding -march=armv7-a -marm -mfpu=neon-vfpv4 -mfloat-abi=$(FLOAT_ABI) -fsigned-char -I$(KERNEL) -DRASPPI=$(RASPPI) -w
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-unwind-tables -fno-rtti
LDFLAGS = -T $(KERNEL)/rpi$(RASPPI).ld -nostartfiles -fno-exceptions -fno-unwind-tables -fno-rtti -Wl,-Map=$(TARGET).map -o $(TARGET).elf
else ifeq ($(strip $(RASPPI)),3)
TARGET=kernel8-32
ASFLAGS = -march=armv8-a -mcpu=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=$(FLOAT_ABI) 
CFLAGS += -fpermissive -Wall -O2 -ffreestanding -march=armv8-a -mtune=cortex-a53 -marm -mfpu=neon-fp-armv8 -mfloat-abi=$(FLOAT_ABI) -fsigned-char -I$(KERNEL) -DRASPPI=$(RASPPI) -w
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-unwind-tables -fno-rtti
LDFLAGS = -T $(KERNEL)/rpi$(RASPPI).ld -nostartfiles -fno-exceptions -fno-unwind-tables -fno-rtti -Wl,-Map=$(TARGET).map -o $(TARGET).elf
else ifeq ($(strip $(RASPPI)),4)
TARGET=kernel7l
ASFLAGS = --warn -mfpu=neon-fp-armv8  -mfloat-abi=$(FLOAT_ABI) -march=armv8-a -mcpu=cortex-a53
CFLAGS += -fpermissive -Wall -O2 -ffreestanding -march=armv8-a -mtune=cortex-a72 -marm -mfpu=neon-fp-armv8 -mfloat-abi=$(FLOAT_ABI) -fsigned-char -I$(KERNEL) -DRASPPI=$(RASPPI) -w
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-unwind-tables -fno-rtti
LDFLAGS = -T $(KERNEL)/rpi$(RASPPI).ld -nostartfiles -fno-exceptions -fno-unwind-tables -fno-rtti -Wl,-Map=$(TARGET).map -o $(TARGET).elf
endif

ifeq ($(USPI),1)
CFLAGS += -DHAVE_USPI
endif

LDFLAGS += 
LIBS += 
LIBS_DEP += 

OS ?=$(shell uname -a)
USER ?=$(shell whoami)


%.o: %.c
	@mkdir -p $(DEPDIR)/$(@D)
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) -std=c99 -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $<
	@mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po

%.o: %.cpp
	@mkdir -p $(DEPDIR)/$(@D)
	@echo "  CXX   $<"
#echo @$(CXX) $(CPPFLAGS) -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $<
	@$(CXX) $(CPPFLAGS) -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $<
	@mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po

%.o: %.s
	@echo "  ASM   $<"
	@$(AS) $(ASFLAGS) -c -o $@ $<

%.o: %.png
	@echo "  PNG   $<"
	$(LD) -r -b binary -o $@ $<

%.o: %.ogg
	@echo "  OGG   $<"
	$(LD) -r -b binary -o $@ $<

%.o: %.txt
	@echo "  TEXT  $<"
	$(LD) -r -b binary -o $@ $<

mount:
ifneq (,$(MOUNTPOINT))
	@echo $(OS) $(MOUNTPOINT)
endif
ifneq (exist,$(shell [ -e $(DISK) ] && echo exist))
	@sudo mkdir $(DISK)
endif
ifneq (,$(findstring Microsoft,$(OS)))
	sudo mount -t drvfs D: $(DISK)
MOUNTPOINT=$(DISK)
else
	sudo fsck -p -y -a $(DEV)
endif
ifneq (,$(wildcard $(DISK)))
ifeq (,$(shell ls $(DISK)))
	@sudo mount $(DEV) $(DISK) -o umask=000
	@sudo chmod +rw $(DISK)
endif
endif

umount:
ifeq (exist,$(shell [ -e $(DISK) ] && echo exist))
ifneq (,$(findstring Microsoft,$(OS)))
	@sudo umount $(DISK)
else
	ifneq ($(shell ls -v $(DISK)),)
		@sudo umount $(DEV)
	endif
endif
endif

mv: $(TARGET).img mount
	@cp $(TARGET).img /mnt/e
	@rm $(TARGET).img
