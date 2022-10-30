#include <stdbool.h>
#include <SDL.h>
#include <SDL_video.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <time.h>
#ifdef WASM
#include <emscripten.h>
#endif 

#define WINDOW_WIDTH    320*2
#define WINDOW_HEIGHT   240*2

const unsigned int size = 64;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

SDL_Point velocity = {0, 0};
SDL_Rect sprite = {0, 0, size, size};
SDL_Rect rect = {0, 0, 320, 240};
SDL_Texture *texture = NULL;
SDL_Texture *pbuf = NULL;

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

    char *bitmap = malloc(320*240);
    SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(bitmap, 320, 240, 8, 320, 0, 0, 0, 0);
    SDL_Color colors[256];
    int i;
    for(i = 0; i < 256; i++)
    {
        colors[i].r = colors[i].g = colors[i].b = (Uint8)i;
    }
    SDL_SetPaletteColors(surf->format->palette, colors, 0, 256);
    pbuf = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
        SDL_TEXTUREACCESS_STREAMING, 320, 240);
    // printf("pbuf:%04x\n", pbuf);
    SDL_SetWindowTitle(window, "SPC-1000 emulator");    

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
    static int fullscreen = SDL_WINDOW_FULLSCREEN;
    SDL_SetWindowFullscreen(window, fullscreen);
    fullscreen = fullscreen ? 0 : SDL_WINDOW_FULLSCREEN;
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
    int *buf;
    int pitch;
    if (!SDL_LockTexture(pbuf, NULL, (void**)&buf, &pitch))
    {
        for (uint32_t i = 0; i < 320 * 240; ++i)
            buf[i] = rand();
        // printf("buf:%04x", buf[0]);
        // Unlock the texture in VRAM to let the GPU know we are done writing to it
        SDL_UnlockTexture(pbuf);
        SDL_RenderCopy(renderer, pbuf, NULL, &rect);
    }
    SDL_RenderCopy(renderer, texture, NULL, &sprite);
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

int main() {
    init();
    load_textures();

    sprite.x = (WINDOW_WIDTH - size) / 2;
    sprite.y = (WINDOW_HEIGHT - size) / 2;

    emscripten_set_main_loop(main_loop, -1, 1);
    destroy();
    return EXIT_SUCCESS;
}
