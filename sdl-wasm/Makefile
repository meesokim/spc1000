all: wasm bin
SOURCE = main.c MC6847.c z80_zazu.c cassette.c AY8910.c Debug.c keyboard.c
EMCC_DEBUG=1
wasm: $(SOURCE)
	emcc $(SOURCE) \
	-O1 \
	-DWASM \
	-s WASM=1 \
	-s USE_SDL=2 \
	-s USE_SDL_IMAGE=2 \
	-s USE_SDL_TTF=2 \
	--memoryprofiler \
	-s SDL2_IMAGE_FORMATS='["png","bmp"]' \
	--preload-file assets \
	-o page.html

bin: $(SOURCE)
	gcc $(SOURCE) -o main -I/usr/include/SDL2 -lc -lSDL2 -lSDL2_ttf -lSDL2_image -DNATIVE 
	
clean:
	rm -f *.js *.wasm *.data main *.html
