#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#include <SDL.h>
#include <SDL_events.h>
//#include <SDL_mixer.h>
#include "cpu.h"
#include "mc6847.h"
#include "ay8910.h"
#include "keyboard.h"
#include "cassette.h"
#include "spcall.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "ugui.h"
#ifdef __cplusplus
}
#endif
#define LOW 0
#define HIGH 1
#define OUTPUT 1
#define INPUT 0

typedef struct _reg {
    char IPLK;
    char motor;
    char pulse;
    char button;
    _reg() { 
        IPLK = 1;
    }
} Registers;

Registers reg;

void pinMode(int pin, int mode)
{
    
}

CMC6847 mc6847;
CKeyboard kbd;
CPU cpu;
AY8910 ay8910;
Cassette cassette;

uint8_t memory[0x10000];

static uint8_t rb(void *userdata, uint16_t addr) {
    uint8_t data;
    data = (reg.IPLK ? ROM[addr & 0x7fff] : memory[addr&0xffff]);
    return data;
}

static void wb(void *userdata, uint16_t addr, uint8_t val) {
    memory[addr&0xffff] = val;
}

static uint8_t in(z80* const z, uint16_t port) {
    uint8_t retval = 0xff;
	if (port >= 0x8000 && port <= 0x8009) // Keyboard Matrix
	{
		return kbd.matrix(port);
	}
	else if ((port & 0xE000) == 0xA000) // IPLK
	{
		reg.IPLK = !reg.IPLK;
	} else if ((port & 0xE000) == 0x2000) // GMODE
	{
		return mc6847.GMODE;
	} else if ((port & 0xE000) == 0x0000) // VRAM reading
	{
		return mc6847.VRAM[port];
	}	
    else if ((port & 0xFFFE) == 0x4000) // PSG
	{
		if (port & 0x01) // Data
		{
			if (ay8910.reg == 14)
			{
				// 0x80 - cassette data input
				// 0x40 - motor status
				// 0x20 - print status
//				if (spcsys.prt.poweron)
//                {
//                    printf("Print Ready Check.\n");
//                    retval &= 0xcf;
//                }
//                else
//                {
//                    retval |= 0x20;
//                }
				if (cassette.motor) //(spcsys.cas.button == CAS_PLAY && spcsys.cas.motor)
				{
					retval &= (~(0x40)); // 0 indicates Motor On
                    cpu.set_turbo(1);
					if (cassette.read(cpu.getCycles(), rb(0, 0x3c5)) == 1)
							retval |= 0x80; // high
						else
							retval &= 0x7f; // low
                    // if (cpu.r->pc == 0x2ba && retval & 0x80)
                    // {
                    //     printf("%d(%d)", retval&0x80 ? 1:0, cpu.r->h);
                    // }
                    // else
                    // {
                    //     printf("0x%04x:%d\n", cpu.r->pc, retval&0x80 ? 1 : 0);
                    // }
                    // if (cassette.pos < 10)
                    // {
                    //     printf("pc:0x%04x\n", cpu.r->pc);
                    //     // cpu.debug();                        
                    // }
				}
				else
					retval |= 0x40;

			}
			else 
			{
				int data = ay8910.read();
				//printf("r(%d,%d)\n", spcsys.psgRegNum, data);
				return data;
			}
		} else if (port & 0x02)
		{
            retval = (cassette.read1() == 1 ? retval | 0x80 : retval & 0x7f);
		}
	}
    // printf("pc:%04x, port:%04x, val:%02x\n", cpu.r->pc, port, retval);
    return retval;
}

static void out(z80* const z, uint16_t port, uint8_t val) {
	if ((port & 0xE000) == 0x0000) // VRAM area
	{
		mc6847.VRAM[port&0x1fff] = val;
	}
	else if ((port & 0xE000) == 0xA000) // IPLK area
	{
		reg.IPLK = !reg.IPLK;	// flip IPLK switch
	}
	else if ((port & 0xE000) == 0x2000)	// GMODE setting
	{
		mc6847.GMODE = val;
	}
	else if ((port & 0xE000) == 0x6000) // SMODE
	{
		// if (reg.button != CASSETTE_STOP)
		{

			if ((val & 0x02)) // Motor
			{
				if (reg.pulse == 0)
				{
					reg.pulse = 1;
				}
			}
			else
			{
				if (reg.pulse)
				{
					reg.pulse = 0;
                    cassette.motor = !cassette.motor;
                    // cpu.set_turbo(cassette.motor);
                    // wb(NULL, 0x3c5, cassette.motor ? 90 : 90);
                    // printf("montor:%d(%d)\n", cassette.motor, rb(0, 0x3c5));
				}
			}
		}
        if (cassette.motor)
        {
            cassette.write(val&1);            
        }
//		if (reg.button == CAS_REC && reg.motor)
//		{
//			CasWrite(&reg, Value & 0x01);
//		}
	}
	else if ((port & 0xFFFE) == 0x4000) // PSG
	{

		if (port & 0x01) // Data
		{
		    if (ay8910.reg == 15) // Line Printer
			{
				if (val != 0)
				{
			    //spcsys.prt.bufs[spcsys.prt.length++] = Value;
				//                    printf("PRT <- %c (%d)\n", Value, Value);
				//                    printf("%s(%d)\n", spcsys.prt.bufs, spcsys.prt.length);
				}
			}
			ay8910.write(val);
            // printf("reg:%d, val:%d\n", ay8910.reg, val);
		}
		else // Reg Num
		{
			ay8910.latch(val);
            // printf("reg:%d,", val);
		}
	}
}

#define CPU_FREQ 4000000
#define PSG_CLOCK PSG_CLOCK_RATE
#define SPC1000_AUDIO_FREQ PSG_CLOCK_RATE
#define SPC1000_AUDIO_BUFFER_SIZE 512

static unsigned ptime;
static unsigned etime;
SDL_AudioSpec audioSpec;
int audid;
bool crt_effect = true;

void audiocallback(
  void* userdata,
  Uint8* stream,
  int    len)
{
    int samples = len / (sizeof(int16_t) * audioSpec.channels);
    Uint16* stream0 = (Uint16 *)stream;
    for(int i = 0; i < len/sizeof(int16_t); i++)
        stream0[i] = ay8910.calc();
}

unsigned int execute(Uint32 interval, void* name)
{
    static int frame = 0;
    etime = SDL_GetTicks();
    cpu.pulse_irq(0);
    int steps = cpu.exec(etime);
    cpu.clr_irq();
    if (frame++%2)
        mc6847.Update();
    ptime = etime;
    return 0;
}

#include <iostream>

SDL_Renderer *renderer;
SDL_Surface *surface;
SDL_Window *screen;
SDL_Texture *texture;
SDL_Texture *texture_title, *texture_display;
UG_COLOR *pixels;
SDL_Event event;
UG_GUI ug;
int w, h;
uint32_t textout_time;
char text[256];
UG_COLOR bgcolor;

void SetPixel(UG_S16 x, UG_S16 y, UG_COLOR color)
{
    if (pixels && color != bgcolor) {
        pixels[x + y * w * 2] = color;
    }
}

void setText(const char *s, int keep_time = 2000)
{
    textout_time = SDL_GetTicks() + keep_time;
    bgcolor = 0xff000000;
    UG_FillFrame(00, 00, 639, 55, 0x0);
    UG_SetForecolor(0xff000000 | C_DARK_GRAY);
    bgcolor = C_BLACK;
    // printf("%s\n", s);
    UG_PutString(11, 15, (char *)s);
    UG_SetForecolor(0xff000000 | C_WHITE);
    UG_PutString(10, 14, (char *)s);
}

void clearText()
{
    // UG_FillFrame(10, 10, 640, 50, 0xff000000 | C_BLACK);
}

static uint32_t last_time = 0;

void reset()
{
    cpu.init();
    cpu.set_turbo(0);
    cpu.set_read_write(rb, wb);
    cpu.set_in_out(in, out);
    mc6847.Initialize();
    ay8910.reset();
    memset(memory, 0, 0x10000);
    reg.IPLK = true;
    last_time = SDL_GetTicks();
}



#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

SDL_Rect dstrect;

void ToggleFullscreen(SDL_Window* window) {
    Uint32 flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
    static int lastWindowX, lastWindowY, width, height;
    bool isFullscreen = SDL_GetWindowFlags(window) & flag;
    if(!isFullscreen){
        SDL_GetWindowPosition(window, &lastWindowX, &lastWindowY);
        SDL_GetWindowSize(window, &width, &height);
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        SDL_SetWindowPosition(window, lastWindowX, lastWindowY);
        SDL_SetWindowFullscreen(window, 0);
        SDL_SetWindowSize(window, width, height);
    }
}

bool ProcessSpecialKey(SDL_Keysym ksym)
{
    bool pressed = false;
	int index = ksym.sym % 256;
    if (ksym.mod & KMOD_ALT)
    {
        switch (ksym.sym)
        {
            case SDLK_LEFT: 
                cassette.prev();
                cassette.get_title(text);
                setText(text);
                break;
            case SDLK_RIGHT:
                cassette.next();
                cassette.get_title(text);
                setText(text);
                break;
            case SDLK_RETURN:
                ToggleFullscreen(screen);
                pressed = true;
                break;
        }
    }
    else {
        switch (ksym.sym)
        {
	    case SDLK_F12:
		exit(0);
		break;
            case SDLK_F8:
                crt_effect = !crt_effect;
                break;
            case SDLK_F10:
                cpu.set_turbo(0);
                reset();
                break;
            case SDLK_F9:
                cpu.set_turbo(0);
                break;
#ifdef __EMSCRIPTEN__                
            case SDLK_F7:
                char msg[100];
                sprintf(msg, "alert('exec:%d한글')", cpu.getCycles());
                emscripten_run_script(msg);
                break;
#endif                
            case SDLK_F6:
                SDL_Event event = {};
                event.type = SDL_KEYDOWN;
                event.key.state = SDL_PRESSED;
                event.key.keysym.sym = SDLK_LSHIFT;
                event.key.keysym.mod = 0; // from SDL_Keymod
                SDL_PushEvent(&event);
                event.key.keysym.sym = SDLK_F1;
                SDL_PushEvent(&event);
                event.type = SDL_KEYUP;
                event.key.state = SDL_RELEASED;
                event.key.keysym.sym = SDLK_F1;
                SDL_PushEvent(&event);
                event.key.keysym.sym = SDLK_LSHIFT;
                SDL_PushEvent(&event);
                break;
        }
    }
    return pressed;
}

#ifdef __EMSCRIPTEN__      
// EMSCRIPTEN_KEEPALIVE
extern "C" {

enum {
    RESET,
    LOAD,
    RUN,
    TAPE_PREV,
    TAPE_NEXT,
    TAPE_SET,
    TAPE_LOAD,
    TURBO,
    SCANLINE
};
void keydown(char *code, bool shift, bool ctrl, bool grp, bool lock, bool single)
{
    // printf("keydown:%s,%d,%d,%d,%d,%d\n", code, shift, ctrl, grp, lock, single);
    kbd.KeyPress(code, shift, ctrl, grp, lock, single);
}
const char * remote(int i, int j, const char *data, const char *filename) {
    //printf("remote:%d\n", i);
    SDL_Event event = {};
    switch (i) {
        case RESET:
            reset();        
            return text;
            break;
        case LOAD:
            event.type = SDL_KEYDOWN;
            event.key.keysym.sym = SDLK_F6;
            // event.key.timestamp = SDL_GetTicks() + 3000;
            SDL_PushEvent(&event);
            break;
        case RUN:
            event.type = SDL_KEYDOWN;
            event.key.keysym.sym = SDLK_F5;
            event.key.state = SDL_PRESSED;
            // event.key.timestamp = SDL_GetTicks() + 3000;
            SDL_PushEvent(&event);
            event.type = SDL_KEYUP;
            event.key.state = SDL_RELEASED;
            SDL_PushEvent(&event);
            break;
        case TAPE_PREV:
            event.type = SDL_KEYDOWN;
            event.key.keysym.sym = SDLK_LEFT;
            event.key.keysym.mod = KMOD_ALT;
            ProcessSpecialKey(event.key.keysym);
            return text;
            // SDL_PushEvent(&event);
            // event.type = SDL_KEYUP;
            // SDL_PushEvent(&event);
            break;
        case TAPE_NEXT:
            event.type = SDL_KEYDOWN;
            event.key.keysym.sym = SDLK_RIGHT;
            event.key.keysym.mod = KMOD_ALT;
            ProcessSpecialKey(event.key.keysym);
            return text;
            // SDL_PushEvent(&event);
            // event.type = SDL_KEYUP;
            // SDL_PushEvent(&event);
            break;
        case TAPE_SET:
            cassette.settape(j);
            cassette.get_title(text);
            setText(text);
            return text;
            break;
        case TAPE_LOAD:
            printf("filename: %s (%d)\n", filename, j);
            cassette.load(data, j, filename);
            cassette.get_title(text);
            setText(text, 5000);
            printf("filename: %s\n", text);
            return text;
            break;
        case TURBO:
            cpu.set_turbo(j>0);
            printf("turbo:%d\n",j>0);
            break;
        case SCANLINE:
            if (j>1)
                crt_effect = ! crt_effect;
            else
                crt_effect = j == 1 ? 1 : 0;
            break;
    }
    return NULL;
}
// The callback:
static bool display_size_changed = false;  // custom global flag
static EM_BOOL on_web_display_size_changed( int event_type, 
  const EmscriptenUiEvent *event, void *user_data )
{
    display_size_changed = true;
    printf("on_web_display_size_changed\n");
    return 0;
}
}


#endif



UG_COLOR crtbuf[640*480];
void  main_loop()
{
    static int i = 1000;
    static auto first_call = true;
    static uint32_t times = 0;
    static double w0, h0;
    SDL_Rect viewport;
    if(first_call)
    {
        int led_status = LOW;
        last_time = SDL_GetTicks();
        reset();
        w = 320; h = 240;
        UG_Init(&ug, SetPixel, w * 2, h * 2);
        UG_FontSelect(&FONT_12X20);
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
        dstrect.w = SCREEN_WIDTH;
        dstrect.h = SCREEN_HEIGHT;
        dstrect.x = dstrect.y = 0;
        SDL_CreateWindowAndRenderer(w * 2, h * 2, SDL_WINDOW_FULLSCREEN_DESKTOP, &screen, &renderer);
        // SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
#ifndef __circle__        
        // SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
#endif
        SDL_RenderGetViewport(renderer, &viewport);
        surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w * 2, h * 2, 32, 0, 0, 0, 0);
        SDL_SetColorKey(surface, SDL_TRUE, 0x0);
        texture_title = SDL_CreateTextureFromSurface(renderer, surface);
        texture_display = SDL_CreateTextureFromSurface(renderer, surface);

        for(int i = 0; i < SCREEN_HEIGHT; i++)
        {
            int color = i%2 ? 0x00000000 : 0x40000000;
            for(int j = 0; j < SCREEN_WIDTH; j++)
            {
                crtbuf[i*640+j] = color;
            }
        }
        SDL_UpdateTexture(texture_display, NULL, crtbuf, w*2*4);
        pixels = (UG_COLOR *)surface->pixels;
        UG_SetBackcolor(C_BLACK);
        // UG_SetForecolor(C_RED);
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, w, h);
        if (!texture) {
            printf("%s\n", SDL_GetError());
        }
        
        //Initialize SDL2 Audio
        SDL_AudioSpec specs = {};
        specs.freq = SPC1000_AUDIO_FREQ;
        specs.format = AUDIO_S16SYS;
        specs.channels = 1;
        specs.samples = 2048;
        specs.callback = audiocallback;
        constexpr int PLAYBACK_DEV = 0;
        audid  = SDL_OpenAudioDevice( nullptr, PLAYBACK_DEV, &specs, &audioSpec, 0 );
        if( audid == 0 )
        {
            std::cerr << "Error opening audio device: " << SDL_GetError() << std::endl;
            return;
        }
        SDL_PauseAudioDevice(audid, 0);
        // pinMode(16, OUTPUT);
        // register_timer(&tw, 250000);
        ptime = SDL_GetTicks();
        ay8910.initTick(ptime);
        cpu.initTick(ptime);
        cassette.initTick(ptime);
        cassette.get_title(text);
        // setText(text);        
        first_call = false;
    }
    SDL_Delay(1000/60);
    uint32_t ticks = SDL_GetTicks();
    execute(ticks - last_time, NULL);
    last_time = ticks;
    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN)
        {
            if (ProcessSpecialKey(event.key.keysym))
                return;
        }
// #ifdef __EMSCRIPTEN__
        else if (event.type == SDL_WINDOWEVENT)
        {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) 
            {
                SDL_Rect  vp;
                SDL_RenderGetViewport(renderer, &vp);
                printf("w=%d,h=%d\n", vp.w, vp.h);
                // if (viewport.w != vp.w || viewport.h != vp.h)
                // {
                //     // auto Width = vp.w;
                //     // auto Height = vp.h;
                //     // dstrect.x = (Width-Height*4/3)/2;
                //     // dstrect.y = 0;
                //     // dstrect.w = Height*4/3;
                //     // dstrect.h = Height; 
                //     Update_Window(screen);              
                //    SDL_RenderGetViewport(renderer, &viewport);    
                // }
            }
        }
// #endif
        kbd.handle_event(event);
    }
    if (times++%2)
    {
        // SDL_Rect  vp;
        // SDL_RenderGetViewport(renderer, &vp);
        // if (vp.h != viewport.h || vp.w != viewport.w)
        // {
        //     printf("w=%d,h=%d\n,", vp.w, vp.h);
        //     viewport.h = vp.h;
        //     viewport.w = vp.w;
        // }
#ifdef __EMSCRIPTEN__
        if (times%100)
        {
            double w, h;
            emscripten_get_element_css_size( "#canvas", &w, &h );
            if (w0 != w || h0 != h)
            {
                // SDL_SetWindowSize( screen, (int)w, (int) h );
                // printf("w=%d,h=%d\n", (int)w, (int) h);
                w0 = w; h0 = h;
            }
            // display_size_changed = true;
        }
#endif
        SDL_UpdateTexture(texture, NULL, mc6847.GetBuffer(), w*2);
        
        int win_w, win_h;
        SDL_GetWindowSize(screen, &win_w, &win_h);
        float aspect = (float)SCREEN_WIDTH / SCREEN_HEIGHT;
        float scale = (float)win_w / SCREEN_WIDTH;
        if ((float)win_h / SCREEN_HEIGHT < scale)
            scale = (float)win_h / SCREEN_HEIGHT;
        
        dstrect.w = (int)(SCREEN_WIDTH * scale);
        dstrect.h = (int)(SCREEN_HEIGHT * scale);
        dstrect.x = (win_w - dstrect.w) / 2;
        dstrect.y = (win_h - dstrect.h) / 2;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, &dstrect);
        if (crt_effect)
            SDL_RenderCopy(renderer, texture_display, NULL, &dstrect); // draws the character
        if (SDL_GetTicks() < textout_time)
        {
            SDL_UpdateTexture(texture_title, NULL, surface->pixels, w*2*4);
            SDL_RenderCopy(renderer, texture_title, NULL, &dstrect);
        }
        SDL_RenderPresent(renderer);
    }
}

#include <sys/stat.h>
int main(int argc, char *argv[]) {

#ifdef TAPE_DIR
    #define TAPE TAPE_DIR
#else
    #define TAPE "taps2"
#endif
    char *tapefile = (char *)TAPE;
    if (argc > 1)
    {
        struct stat sb;
        if (!stat(argv[1], &sb))
            tapefile = argv[1];
    }
    printf("tapefile:%s\n", tapefile);
    cassette.setfile(tapefile);
    // struct timer_wait tw;

#ifdef __EMSCRIPTEN__    
    emscripten_set_resize_callback(
        EMSCRIPTEN_EVENT_TARGET_WINDOW,
        0, 0, on_web_display_size_changed
    );
    emscripten_set_main_loop(main_loop, 30, 1);
#else
    // cpu.set_breakpoint(0x27);
    do {
        main_loop();
    }
    while(event.type != SDL_QUIT);
#endif
}
