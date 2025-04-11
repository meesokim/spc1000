all: wasm bin circle
CXXSOURCES = main.cpp mc6847.cpp keyboard.cpp cpu.cpp cassette.cpp
CSOURCES = spcall.c emu2149.c ugui.c miniz/miniz.c miniz/miniz_zip.c miniz/miniz_tdef.c miniz/miniz_tinfl.c
SOURCES = $(CXXSOURCES) $(CSOURCES)
OBJECTS=$(CXXSOURCES:.cpp=.o) $(CSOURCES:.c=.o)
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
	arm-none-eabi-g++ $(SOURCES) -D__circle__ -ISDL2 -I../bzip2 -fpermissive -Iminiz bzlib.o ~/circle/lib/libcircle.a -lc libSDL2.a -DDIR=\"$(DIR)\" -o kernel.elf

pub:
	cp index.* ~/spc1000.wiki/docs
	
clean:
	rm -f *.js miniz/*.o *.o *.wasm *.data main 
