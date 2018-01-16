set PATH=.\bin;%PATH%
z80asm -fb -l -sa -ocpm.bin spc-1000_cpm22.asm 
z80asm -fb -l -sa -oboot.bin spc-1000_cpm_boot.asm 
z80asm -fb -l -sa -obios.bin spc-1000_cpm_bios.asm 
set CPMTOOLSFMT=sd725
set DISKFILE="system.dsk"
del %DISKFILE%
if not exist %DISKFILE% vformat %DISKFILE%
for %%f in (cpm22\*.*) do cpmcp -f sd725 -T dsk %DISKFILE% cpm22/%%~nxf 0:%%~nxf
rem for %%f in (cpm22\*.*) do vfwrite %DISKFILE% cpm22/%%~nxf %%~nxf
putsys2 %DISKFILE%
move %DISKFILE% ..\disk