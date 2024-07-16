all: wasm bin
CXXSOURCES = main.cpp mc6847.cpp keyboard.cpp cpu.cpp cassette.cpp
CSOURCES = spcall.c emu2149.c ugui.c
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
	$(CXX) $(CFLAGS) -c $< -o $@

wasm: $(OBJECTS)
	emcc $(OBJECTS) \
	-DWASM \
	-s WASM=1 \
	-sUSE_SDL=2 -sUSE_BZIP2 \
	--preload-file $(DIR) \
	-sALLOW_MEMORY_GROWTH \
	-sEXPORTED_FUNCTIONS=_remote,_main -sEXPORTED_RUNTIME_METHODS=ccall,cwrap \
	-o index.js 
bin: $(SOURCES)
	gcc $(SOURCES) -o main -I/usr/include/SDL2 -lc -lSDL2 -lSDL2_ttf -lstdc++ -lbz2 -lzip -DDIR=\"$(DIR)\"
	
clean:
	rm -f *.js *.o *.wasm *.data main 
