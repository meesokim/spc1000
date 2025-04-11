all: wasm bin circle
CXXSOURCES = main.cpp mc6847.cpp keyboard.cpp cpu.cpp cassette.cpp
CSOURCES = spcall.c emu2149.c ugui.c miniz/miniz.c miniz/miniz_zip.c miniz/miniz_tdef.c miniz/miniz_tinfl.c
PSOURCES = $(CSOURCES) bzlib.c crctable.c
SOURCES = $(CXXSOURCES) $(CSOURCES)
OBJECTS=$(CXXSOURCES:.cpp=.o) $(CSOURCES:.c=.o)
POBJECTS = $(CXXSOURCES:.cpp=.o) $(notdir $(PSOURCES:.c=.o))

EMCC_DEBUG=1
DIR?=taps
CFLAGS = -O2 -sUSE_SDL=2 -sUSE_BZIP2 -DDIR=\"$(DIR)\"
CC=emcc
CXX=em++
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ -Iminiz

wasm: $(OBJECTS)
	emcc $(OBJECTS) \
	-DWASM \
	-s WASM=1 -sUSE_WEBGL2=1 \
	-sUSE_SDL=2 -sUSE_BZIP2 \
	-sSTACK_SIZE=8192000 \
	--preload-file $(DIR) \
	-sALLOW_MEMORY_GROWTH \
	-sEXPORTED_FUNCTIONS=_remote,_keydown,_main -sEXPORTED_RUNTIME_METHODS=ccall,cwrap \
	-o index.js 
bin: $(SOURCES)
	gcc $(SOURCES) -o main -I/usr/include/SDL2 -Iminiz -lc -lSDL2 -lstdc++ -lbz2 -lzip -DDIR=\"$(DIR)\"

circle: $(SOURCES)
	arm-none-eabi-gcc $(PSOURCES) -c -Wall -O2 -ffreestanding -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -fsigned-char -D__circle__ -ISDL2 -I../bzip2 -Iminiz 
	arm-none-eabi-g++ $(CXXSOURCES) -c -Wall -O2 -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -fsigned-char -D__circle__ -ISDL2 -I../bzip2 -fpermissive -Iminiz 
	arm-none-eabi-g++ $(POBJECTS) -T raspberry.ld -nostartfiles -fno-exceptions -fno-unwind-tables -fno-rtti -Wl,-Map=kernel.map bzlib.o crctable.o libkernel.a -lc libSDL2.a -o kernel.elf

pub:
	cp index.* ~/spc1000.wiki/docs
	
clean:
	rm -f *.js miniz/*.o *.o *.wasm *.data main 
