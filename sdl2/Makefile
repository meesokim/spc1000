all: wasm bin circle

CXXSOURCES = main.cpp mc6847.cpp keyboard.cpp cpu.cpp cassette.cpp
CSOURCES = spcall.c emu2149.c ugui.c miniz/miniz.c miniz/miniz_zip.c miniz/miniz_tdef.c miniz/miniz_tinfl.c
PSOURCES = $(CSOURCES) bzlib.c crctable.c

DIR?=taps

# Compiler selection based on target
WASM_CC = emcc
WASM_CXX = em++
BIN_CC = gcc
BIN_CXX = g++
CIRCLE_CC = arm-none-eabi-gcc
CIRCLE_CXX = arm-none-eabi-g++

# Target specific rules
%.wasm.o: %.c
	$(WASM_CC) -O2 -sUSE_SDL=2 -sUSE_BZIP2 -DDIR=\"$(DIR)\" -c $< -o $@

%.wasm.o: %.cpp
	$(WASM_CXX) -O2 -sUSE_SDL=2 -sUSE_BZIP2 -DDIR=\"$(DIR)\" -c $< -o $@ -Iminiz

%.bin.o: %.c
	$(BIN_CC) -I/usr/include/SDL2 -Iminiz -O2 -DDIR=\"$(DIR)\" -c $< -o $@

%.bin.o: %.cpp
	$(BIN_CXX) -I/usr/include/SDL2 -Iminiz -O2 -DDIR=\"$(DIR)\" -c $< -o $@ -Iminiz

%.circle.o: %.c
	$(CIRCLE_CC) -c -Wall -O2 -ffreestanding -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -fsigned-char -D__circle__ -ISDL2 -I../bzip2 -Iminiz $< -o $@

%.circle.o: %.cpp
	$(CIRCLE_CXX) -c -Wall -O2 -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -fsigned-char -D__circle__ -ISDL2 -I../bzip2 -fpermissive -Iminiz $< -o $@

WASM_OBJECTS = $(CXXSOURCES:.cpp=.wasm.o) $(CSOURCES:.c=.wasm.o)
BIN_OBJECTS = $(CXXSOURCES:.cpp=.bin.o) $(CSOURCES:.c=.bin.o)
CIRCLE_OBJECTS = $(CXXSOURCES:.cpp=.circle.o) $(PSOURCES:.c=.circle.o)

wasm: $(WASM_OBJECTS)
	$(WASM_CC) $(WASM_OBJECTS) -DWASM -s WASM=1 -sUSE_WEBGL2=1 -sUSE_SDL=2 -sUSE_BZIP2 -sSTACK_SIZE=8192000 --preload-file $(DIR) -sALLOW_MEMORY_GROWTH -sEXPORTED_FUNCTIONS=_remote,_keydown,_main -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -o index.js 

bin: $(BIN_OBJECTS)
	$(BIN_CXX) $(BIN_OBJECTS) -o main -lc -lSDL2 -lstdc++ -lbz2 -lzip

circle: $(CIRCLE_OBJECTS)
	$(CIRCLE_CXX) $(CIRCLE_OBJECTS) -T raspberry.ld -nostartfiles -fno-exceptions -fno-unwind-tables -fno-rtti -Wl,-Map=kernel.map bzlib.o crctable.o libkernel.a -lc libSDL2.a -o kernel.elf

clean:
	rm -f *.js miniz/*.o *.o *.wasm *.data main kernel.elf *.bin.o *.wasm.o *.circle.o

pub:
	cp index.* ~/spc1000.wiki/docs
