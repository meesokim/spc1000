sdasz80 -l -o boot.s
sdcc boot.rel
hex2bin boot.ihx
putsys2 system.d88
