sdasz80 -l -o boot.s
sdcc -mz80 --no-std-crt0 boot.rel
