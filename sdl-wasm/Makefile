all: sdl-wasm bin
	
sdl-wasm:
	emcc main.c \
	-DWASM \
	-I/usr/include/SDL2 \
	-s WASM=1 \
	-s USE_SDL=2 \
	-s USE_SDL_IMAGE=2 \
	-s SDL2_IMAGE_FORMATS='["png","bmp"]' \
	--preload-file assets \
	-o index.js

bin:
	gcc main.c -o main -I/usr/include/SDL2 -lSDL2 -lSDL2_ttf -lSDL2_image -DNATIVE

	
clean:
	rm -f *.js *.wasm *.data main
