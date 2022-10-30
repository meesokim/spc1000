#include <stdbool.h>
#include <SDL.h>
#include <SDL_video.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <time.h>
#ifdef WASM
#include <emscripten.h>
#endif 

#include "spcall.rom.h"
#include "MC6847.h"

#define WINDOW_WIDTH    320*2
#define WINDOW_HEIGHT   240*2

const unsigned int size = 64;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

SDL_Point velocity = {0, 0};
SDL_Rect sprite = {0, 0, size, size};
SDL_Rect rect = {0, 0, 320, 240};
SDL_Rect rectx2 = {0, 0, 640, 480};
SDL_Texture *texture = NULL;
SDL_Texture *scrtex = NULL;
SDL_Surface *surface;
char *pixels;
int w = 320, h = 240;

SDL_Color colors[] = {_CLR0, _CLR1, _CLR2, _CLR3, _CLR4, _CLR5, _CLR6, _CLR7, _CLR8, _CLR9, _CLR10, _CLR11, _CLR12, _CLR13, _CLR14};

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER) < 0) {
        return false;
    }
    if (TTF_Init() < 0)
    {
        printf("Could not initialize SDL_tff: %s", SDL_GetError());
        exit(-2);
    }
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);
    if (window == NULL | renderer == NULL) {
        return false;
    } 
    SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    // pixels = malloc(320*240);
    // SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(bitmap, 320, 240, 8, 320, 0, 0, 0, 0);
    // SDL_Surface *surf = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 8, 0,0,0,0);
    // stex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_INDEX8, SDL_TEXTUREACCESS_STREAMING, 320, 240);
    // surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
    // surface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, w, h, 8, SDL_PIXELFORMAT_INDEX8);
    scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);    
    // scrtex = SDL_CreateTextureFromSurface(renderer, surface);
    // SDL_Palette *palette = malloc(sizeof(SDL_Palette));
    // palette->colors = colors;
    // palette->ncolors = sizeof(colors)/sizeof(SDL_Color);
    // SDL_SetSurfacePalette(surface, palette);
    // printf("ncolors:%d\n", palette->ncolors);
    // printf("scrtex:%x\n", scrtex);
    // printf("surface:%x\n", surface);
    // printf("surface->pixels:%x\n", surface->pixels);
    // ->colors = colors;
    // SDL_SetPaletteColors(surface->format->palette, colors, 0, 2);
    // SDL_Color *pal;
	// if (surf != 0 && surf->format->palette != NULL)
	// {
    //     SDL_Palette *p = malloc(sizeof(SDL_Palette));
    //     p->ncolors = 15;
    //     SDL_Color *pal = p->colors = (SDL_Color *) malloc(sizeof(SDL_Color) * 256);
	// 	pal[0] = (SDL_Color) _CLR0;
	// 	pal[1] = (SDL_Color) _CLR1;
	// 	pal[2] = (SDL_Color) _CLR2;
	// 	pal[3] = (SDL_Color) _CLR3;
	// 	pal[4] = (SDL_Color) _CLR4;
	// 	pal[5] = (SDL_Color) _CLR5;
	// 	pal[6] = (SDL_Color) _CLR6;
	// 	pal[7] = (SDL_Color) _CLR7;
	// 	pal[8] = (SDL_Color) _CLR8;
	// 	pal[9] = (SDL_Color) _CLR9;
	// 	pal[10] = (SDL_Color) _CLR10;
	// 	pal[11] = (SDL_Color) _CLR11;
	// 	pal[12] = (SDL_Color) _CLR12;
	// 	pal[13] = (SDL_Color) _CLR13;
	// 	pal[14] = (SDL_Color) _CLR14;
	// 	SDL_SetSurfacePalette(surf, p);
    //     stex = SDL_CreateTextureFromSurface(renderer, surf);
	// 	// SDL_SetPalette(surf, SDL_LOGPAL, pal, 0, 14);
	// }    
    // printf("stex:%x\n", (unsigned int)stex);
    // SDL_Color colors[256];
    // int i;
    // for(i = 0; i < 256; i++)
    // {
    //     colors[i].r = colors[i].g = colors[i].b = (Uint8)i;
    // }
    // SDL_SetPaletteColors(surf->format->palette, colors, 0, 256);
    // pbuf = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
    //     SDL_TEXTUREACCESS_STREAMING, 320, 240);
    // printf("pbuf:%04x\n", pbuf);
    SDL_SetWindowTitle(window, "SPC-1000 emulator");    
    // SDL_RendererInfo info = {};
    // if(SDL_GetRenderDriverInfo(0, &info) == 0) {
    //     bool supported = false;
    //     for(int i = 0; i < info.num_texture_formats; ++i) {
    //         if(info.texture_formats[i] == SDL_PIXELFORMAT_INDEX8) {
    //             supported = true;
    //             break;
    //         }
    //     }
    //     printf("Supported: %s\n", supported ? "YES" : "NO");
    // }
    // uint32_t format = SDL_PIXELFORMAT_UNKNOWN;
    // SDL_QueryTexture(scrtex, &format, NULL, NULL, NULL);
    // printf("Texture format: %s\n", SDL_GetPixelFormatName(format));    
    return true;
}

void load_textures() {
    SDL_Surface *surface = IMG_Load("assets/hello_world.bmp");
    if (!surface) {
        printf("%s\n", IMG_GetError());
    }
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0x75, 0xFF, 0xFF));
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}
void ToggleFullScreen()
{
    static int fullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP;
    SDL_SetWindowFullscreen(window, fullscreen);
    SDL_DisplayMode desktopMode;
    if ( SDL_GetDesktopDisplayMode(0, &desktopMode) == 0 )
        SDL_SetWindowDisplayMode(window, &desktopMode);
    fullscreen = fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP;
    // printf("%d\n", fullscreen);
}
void process_event(SDL_Event *event) {
    SDL_Keycode key = event->key.keysym.sym;
    // printf("%d\n", key);
    if (event->key.type == SDL_KEYDOWN) {
        if (key == SDLK_LEFT || key == SDLK_RIGHT) {
            velocity.x = key == SDLK_LEFT ? -1 : 1;
        }
        if (key == SDLK_UP || key == SDLK_DOWN) {
            velocity.y = key == SDLK_UP ? -1 : 1;
        }
        if (key == SDLK_F11) {
            ToggleFullScreen();
        }
    }
    if (event->key.type == SDL_KEYUP) {
        velocity.x = 0;
        velocity.y = 0;
    }
}

// Event loop exit flag
bool quit = false;

void process_input() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
        {
            quit = 1;
        }
        process_event(&event);
    }
}

void update() {
    sprite.x += velocity.x;
    sprite.y += velocity.y;
}

void draw() {
    uint32_t *pixels;
    //  = surface->pixels;
    int pitch;
    SDL_LockTexture(scrtex, NULL, (void **)&pixels, &pitch);
    for(uint32_t* p = pixels; p != &pixels[w * h] ; p++) {
        *p = ((uint32_t *)colors)[rand()%16];
    }
    SDL_UnlockTexture(scrtex);
    // SDL_UpdateTexture(scrtex, NULL, pixels, w);
    SDL_RenderCopy(renderer, scrtex, NULL, &rectx2);
    SDL_RenderCopy(renderer, texture, NULL, &sprite);
    // SDL_BlitSurface(surface, NULL, SDL_GetWindowSurface(window), &rectx2);
    SDL_RenderPresent(renderer);    
    // SDL_UpdateWindowSurface(window);
    // SDL_Delay(100);
    // if (!ret) {
    //     // SDL_LockTexture(pbuf);
    //     for (uint32_t i = 0; i < 320 * 240; ++i)
    //         buf[i] = rand();
    //     // printf("buf:%04x", buf[0]);
    //     // Unlock the texture in VRAM to let the GPU know we are done writing to it
    //     SDL_UnlockTexture(scrtex);
    //     SDL_RenderCopyEx(renderer, scrtex, &rect, &rectx2, 0, NULL, SDL_FLIP_NONE);
    // }
    // else
    // {
    //     printf("Error:%s\n", SDL_GetError());
    // }
}

void main_loop() {
    process_input();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    update();
    draw();    

    SDL_RenderPresent(renderer);
}

void destroy() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

#ifndef WASM
void emscripten_set_main_loop(void (*loop)(), int x, int y )
{
    while(!quit)
    {    
        loop();
    }
}
#endif 

typedef unsigned char byte;

byte ROM[0x8000];
byte RAM[0x10000];

void load_rom()
{
    memcpy(ROM, spcall, 0x8000);
}

int main() {
    init();
    load_textures();
    load_rom();
    sprite.x = (WINDOW_WIDTH - size) / 2;
    sprite.y = (WINDOW_HEIGHT - size) / 2;

    emscripten_set_main_loop(main_loop, -1, 1);
    destroy();
    return EXIT_SUCCESS;
}
