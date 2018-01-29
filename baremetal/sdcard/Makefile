#
# Download the firmware files to be required for boot (requires wget)
#
# These files must be copied along with the generated kernel.img
#     onto a SD(HC) card with FAT file system.
#

all: LICENCE.broadcom bootcode.bin fixup.dat start.elf kernel8-32.img


LICENCE.broadcom:
	@echo downloading.. $@ 
	@wget -q -O LICENCE.broadcom https://github.com/raspberrypi/firmware/blob/master/boot/LICENCE.broadcom?raw=true
	
bootcode.bin:
	@echo downloading.. $@ 
	@wget -q -O bootcode.bin https://github.com/raspberrypi/firmware/blob/master/boot/bootcode.bin?raw=true

fixup.dat:
	@echo downloading.. $@ 
	@wget -q -O fixup.dat https://github.com/raspberrypi/firmware/blob/master/boot/fixup.dat?raw=true

start.elf:
	@echo downloading.. $@ 
	@wget -q -O start.elf https://github.com/raspberrypi/firmware/blob/master/boot/start.elf?raw=true
	
kernel8-32.img:
	@rm ../*.o
	$(MAKE) -C .. RASPPI=3
	cp ../$@ .

kernel7.img:
	@rm ../*.o
	$(MAKE) -C .. RASPPI=2
	cp ../$@ .

kernel.img:
	@rm ../*.o
	$(MAKE) -C .. RASPPI=1
	cp ../$@ .
	
clean:
	rm -f bootcode.bin fixup.dat start.elf LICENCE.broadcom

