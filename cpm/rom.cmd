bin\z80asm -fb -l -sa spc-1000_all.asm 
bin\z80asm -fb -l -sa spc-1000_ipl.asm
copy /b spc-1000_all.bin+spc-1000_ipl.bin spcall.rom
@rem copy spcall.rom ..\Debug
bin\zip ../roms/spc1000.zip spcall.rom 