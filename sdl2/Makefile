all: wasm bin
CXXSOURCES = main.cpp mc6847.cpp keyboard.cpp cpu.cpp cassette.cpp
CSOURCES = spcall.c emu2149.c ugui.c
SOURCES = $(CXXSOURCES) $(CSOURCES)
OBJECTS=$(CXXSOURCES:.cpp=.o) $(CSOURCES:.c=.o)
EMCC_DEBUG=1
CFLAGS = -sUSE_SDL=2
CC=emcc
CXX=em++
%.o: %.c
	$(CC) $(CFLAGS) -c "$<" -o "$@"

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

wasm: $(OBJECTS)
	echo $(OBJECTS)
	emcc $(OBJECTS) \
	-O2 \
	-std=c++11 \
	-DWASM \
	-s WASM=1 \
	-s USE_SDL=2 \
	--preload-file ../tape/taps \
	-s SDL2_IMAGE_FORMATS='["png","bmp"]' \
	-o index.html

bin: $(SOURCES)
	gcc $(SOURCES) -o main -I/usr/include/SDL2 -lc -lSDL2 -lSDL2_ttf -lstdc++
	
clean:
	rm -f *.js *.wasm *.data main *.html
