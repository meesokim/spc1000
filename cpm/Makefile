ASM=../bin/z80asm
MKFS=mkfs.cpm
CPMCP=cpmcp

all: system.dsk cpm.bin boot.bin bios.bin

cpm.bin: spc-1000_cpm22.asm
	$(ASM) -fb -l -sa -o$@ $<

boot.bin: spc-1000_ipl.asm
	$(ASM) -fb -l -sa -o$@ $<

bios.bin: spc-1000_cpm_bios.asm
	$(ASM) -fb -l -sa -o$@ $<

system.dsk: cpm.bin boot.bin bios.bin
	$(MKFS) -f sd725 $@
	$(CPMCP) -f sd725 $@ cpm22/*.com 0:

clean:
	rm system.dsk
