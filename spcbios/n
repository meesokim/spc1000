#!/bin/bash
make
sdasz80 -l -o bootpi.s
sdcc bootpi.rel --data-loc 0xce00 --code-size 256
./hex2bin bootpi.ihx
./makesize bootpi.bin 256 > boot.bin
cat boot.bin sd720.bin > spc1000.bin
cp spc1000.bin /e
