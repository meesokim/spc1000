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
#include "common.h"
#include "cassette.h"
#include "keyboard.h"

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
PIXEL *buf;

SPCConfig spconf;
SPCSystem spcsys;
SPCSimul  spcsim;

extern byte *z80mem;

#define FCLOSE(x)	fclose(x),(x)=NULL
#define I_PERIOD 4000
#define TURBO (spconf.turbo) 
#define I_PERIOD_TURBO (I_PERIOD * (TURBO + 1))
#define INTR_PERIOD 16.6666


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
    scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
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
        ProcessSpecialKey(event->key.keysym);
        ProcessKeyDown(event->key.keysym.sym);        
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
        ProcessKeyUp(event->key.keysym.sym);
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

void draw(PIXEL *fb) {
    uint32_t *pixels;
    int pitch;
    SDL_LockTexture(scrtex, NULL, (void **)&pixels, &pitch);
    for(uint32_t* p = pixels; p != &pixels[w * h] ; p++) {
        *p = ((uint32_t *)colors)[*fb++];
    }
    SDL_UnlockTexture(scrtex);
    SDL_RenderCopy(renderer, scrtex, NULL, &rectx2);
    SDL_RenderCopy(renderer, texture, NULL, &sprite);
    SDL_RenderPresent(renderer);    
}

extern unsigned short breakpoint[10];
extern int bp;

void CheckHook(register Z80 *R)
{
    unsigned short i = 0;
    int debug = 0;
    int bpcheck = 0;
    if (bp > 0)
    {
        for (i = 0; i < 20; i++)
        {
            if (breakpoint[i] != 0) {
                bpcheck++;
                if (R->PC.W == breakpoint[i] || R->Trace == 1)
                {
                    debug = 1;
                    break;
                }
            }
            if (bpcheck >= bp)
                break;
        }
    }
    if (debug || spconf.debug)
    {
        spconf.debug = DebugZ80(R);
    }
}

#include <time.h>
long timeGetTime()
{
    struct timespec tspec;
    clock_gettime(CLOCK_MONOTONIC, &tspec);
    return tspec.tv_sec * 1000 + tspec.tv_nsec/1.0e6;
}

void execute()
{
    Z80 *R = &spcsys.Z80R;
    static int tick = 0;
    static int count = 0;
    int t = timeGetTime();
    if (spconf.debug || bp > 0)
        CheckHook(R);	// Special Processing, if needed

    if (R->ICount <= 0)	// 1 msec counter expired
    {
        tick++;		// advance spc tick by 1 msec
        spcsys.tick += (TURBO + 1);
        R->ICount += I_PERIOD;//_TURBO;	// re-init counter (including compensation)

        if (tick % 26 == 0)	// 1/60 sec interrupt timer expired
        {
            if (R->IFF & IFF_EI)	// if interrupt enabled, call Z-80 interrupt routine
            {
                R->IFF |= IFF_IM1 | IFF_1;
                IntZ80(R, 0);
            }
        }
        if (tick % 33)			// check refresh timer
        {
            // SDL_LockSurface(vdpsf);
            Update6847(spcsys.GMODE, &spcsys.VRAM[0], buf);
            draw(buf);
            // CheckKeyboard();
        }
        Loop8910(&spcsys.ay8910, 1);
        int d = timeGetTime() - t;
        if (d > 100) 
            t = timeGetTime(); 
        else
        {
            while(t>timeGetTime() && !TURBO);
            t++;
        }
    }
    count = R->ICount;
    ExecZ80(R); // Z-80 emulation
    spcsys.cycles += (count - R->ICount);    
}
void main_loop() {
    process_input();
    execute();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    update();
    // draw();    

    // SDL_RenderPresent(renderer);
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

#define SPC_EMUL_VERSION "1.2 (2014.04.22)"

void taps_menu() 
{

}

void load_rom()
{
    memcpy(spcsys.ROM, spcall, 0x8000);
}

int main() {
    init();
    load_textures();
    load_rom();
	buf = InitMC6847(); // Tells VRAM address to MC6847 module and init
    ResetZ80(&spcsys.Z80R);
    spcsys.Z80R.ICount = I_PERIOD;
	spcsys.cycles = 0;
	spcsys.tick = 0;
	OpenSoundDevice();
	BuildKeyHashTab();	// Init keyboard hash table
	spcsim.baseTick = SDL_GetTicks();
	spcsim.prevTick = spcsim.baseTick;
	spcsys.intrTime = INTR_PERIOD;
	spcsys.tick = 0;
	spcsys.refrTimer = 0;	// timer for screen refresh
	spcsys.refrSet = spconf.frameRate;	// init value for screen refresh timer

    sprite.x = (WINDOW_WIDTH - size) / 2;
    sprite.y = (WINDOW_HEIGHT - size) / 2;

    emscripten_set_main_loop(main_loop, -1, 1);
    destroy();
    return EXIT_SUCCESS;
}

int OpenTapeFile(void)
{
    int t = 0;
    if (strlen((const char*)spconf.current_tap)>0)
    {

		if ((spconf.rfp = fopen((const char *)spconf.current_tap ,"rb")) == NULL)
		{
			printf("File(%s) (read) open error.\n", spconf.current_tap);
			return -1;
		}
		while(fgetc(spconf.rfp)=='0');
		t = ftell(spconf.rfp);
		fseek(spconf.rfp, (t > 10 ? t - 10 : t) , 0);
		printf("File(%s) (read) opened. %ld\n", spconf.current_tap, ftell(spconf.rfp));
		return 0;
    }
    else
        taps_menu();
}

int SaveAsTapeFile(void)
{
	char szFile[256];
	{
		strcpy(szFile, (const char*)spconf.current_tap);
#ifdef DEBUG_MODE
		printf("Writing %s\n", szFile);
#endif
		if (spconf.wfp != NULL)
			fclose(spconf.wfp);
		if ((spconf.wfp = fopen((const char*)szFile,"wb")) == NULL)
		{
			printf("File(%s) (write) open error.\n", szFile);
			return -1;
		}
		printf("File(%s) (write) created.\n", szFile);
		return 0;
	}
	printf("File save canceled.\n");
	return -1;
}

/*************************************************************/
/** Output I/O Processing                                   **/
/*************************************************************/

/**
 * Put a value on Z-80 out port
 * @param Port 0x0000~0xffff port address
 * @param Value a byte value to be written
 * @warning modified to accomodate 64K I/O address from original Marat's source
 */
int fcno = 0;
char dfile[256];
void OutZ80(register word Port,register byte Value)
{
    //printf("0h%04x < 0h%02x\n", Port, Value);

	if (Port < 0x2000) // VRAM area
	{
		spcsys.VRAM[Port] = Value;
	}
	else if ((Port & 0xE000) == 0xA000) // IPLK area
	{
		spcsys.IPLK = (spcsys.IPLK)? 0 : 1;	// flip IPLK switch
		if (spcsys.IPLK)
            z80mem = spcsys.ROM;
        else
            z80mem = spcsys.RAM;
	}
	else if ((Port & 0xE000) == 0x2000)	// GMODE setting
	{
	    //printf("GMODE=%d\n", Value);
		spcsys.GMODE = Value;
#ifdef DEBUG_MODE
		printf("GMode:%02X\n", Value);
#endif
	}
	else if ((Port & 0xE000) == 0x6000) // SMODE
	{
		if (spcsys.cas.button != CAS_STOP)
		{

			if ((Value & 0x02)) // Motor
			{
				if (spcsys.cas.pulse == 0)
				{
					spcsys.cas.pulse = 1;

				}
			}
			else
			{
				if (spcsys.cas.pulse)
				{
					spcsys.cas.pulse = 0;
					if (spcsys.cas.motor)
					{
						spcsys.cas.motor = 0;
#ifdef DEBUG_MODE
						printf("Motor Off\n");
#endif
					}
					else
					{
						spcsys.cas.motor = 1;
#ifdef DEBUG_MODE
						printf("Motor On\n");
#endif
						spcsys.cas.lastTime = 0;
						ResetCassette(&spcsys.cas);
					}
				}
			}
		}

		if (spcsys.cas.button == CAS_REC && spcsys.cas.motor)
		{
			CasWrite1(&spcsys.cas, Value & 0x01);
		}
	}
	else if ((Port & 0xFFFE) == 0x4000) // PSG
	{

		if (Port & 0x01) // Data
		{
		    if (spcsys.psgRegNum == 15) // Line Printer
            {
                if (Value != 0)
                {
                    spcsys.prt.bufs[spcsys.prt.length++] = Value;
//                    printf("PRT <- %c (%d)\n", Value, Value);
//                    printf("%s(%d)\n", spcsys.prt.bufs, spcsys.prt.length);
                }
            }
			//printf("reg:%d,%d\n", spcsys.psgRegNum, Value);
			Write8910(&spcsys.ay8910, (byte) spcsys.psgRegNum, Value);
		}
		else // Reg Num
		{
			spcsys.psgRegNum = Value;
			WrCtrl8910(&spcsys.ay8910, Value);
		}
	}
	else if ((Port & 0x4003) == 0x4003)
	{
		if (fcno < 256)
		{
			dfile[fcno++] = (Value == 0xff ? 0 : Value);
		}
		if (Value == 0)
		{
			fcno = 0;
			FCLOSE(spconf.rfp);
			sprintf((char *)spconf.current_tap, "..\\tape\\%s.tap", dfile);
			printf("%s\n", (char *)spconf.current_tap);
            if (OpenTapeFile() < 0)
                return;
            spcsys.cas.button = CAS_PLAY;
            spcsys.cas.motor = 1;
            spcsys.cas.lastTime = 0;
            ResetCassette(&spcsys.cas);
            printf("file found\n");
		}
		else if (Value == 0xff)
		{
			fcno = 0;
			FCLOSE(spconf.wfp);
			sprintf((char *)spconf.current_tap, "..\\tape\\%s.tap", dfile);
            if (SaveAsTapeFile() < 0)
                return;
            spcsys.cas.button = CAS_REC;
            spcsys.cas.motor = 1;
            spcsys.cas.lastTime = 0;
            ResetCassette(&spcsys.cas);
            printf("file created\n");
		}
	}
	else if (Port & 0x4004)
	{
		if (spconf.wfp)
		{
			printf("%02x", Value);
			fputc('1', spconf.wfp);
			for(int i = 0; i < 8; i++)
				fputc((Value >> (7 - i) & 1) + '0', spconf.wfp);
		}
	}
	else if (Port & 0x4005)
	{
		if (spconf.wfp)
		{
			FCLOSE(spconf.wfp);
		}
		printf("spconf.wfp closed\n");
	}
	else if ((Port & 0xFFF0) == 0xC000)
    {
        if (Port == 0xc000) // PortA
        {
            spcsys.fdd.wrVal = Value;
            //printf("FDD_Data < 0h%02x\n", Value);
        }
        else if (Port == 0xc002) // PortC
        {
            //printf("FDD_PC < 0h%02x\n", Value);
            if (Value != 0)
            {
                spcsys.fdd.PC.b = (Value & 0xf0) | (spcsys.fdd.PC.b & 0xf);
            }
            else
            {
                spcsys.fdd.PC.b = spcsys.fdd.PC.b & 0xf;
            }
            if (Value & 0x10) // DAV=1
            {
                //printf("FDD Data Valid (c000 bit4 on)\n");
                if (spcsys.fdd.isCmd == 1) // command
                {
                    spcsys.fdd.isCmd = 0;
                    printf("FDD Command(Data) = 0h%02x\n", spcsys.fdd.wrVal);
                    spcsys.fdd.cmd = spcsys.fdd.wrVal;
                    switch (spcsys.fdd.wrVal)
                    {
                        case 0x00: // FDD Initialization
                            printf("*FDD Initialization\n");
                            break;
                        case 0x01: // FDD Write
                            printf("*FDD Write\n");
                            spcsys.fdd.rSize = 4;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x02: // FDD Read
                            printf("*FDD Read\n");
//                            spcsys.fdd.PC.bits.rRFD = 1;
                            spcsys.fdd.rSize = 4;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x03: // FDD Send Data
                            //printf("*FDD Send Data\n");
                            if (spcsys.fdd.seq == 4)
                            {
                                printf("seq=%d,(%d,%d,%d,%d)\n", spcsys.fdd.seq, spcsys.fdd.data[0], spcsys.fdd.data[1], spcsys.fdd.data[2], spcsys.fdd.data[3]);
                                spcsys.fdd.buffer = (byte*)(spcsys.fdd.data[1] != 0 ? spcsys.fdd.diskdata2 : spcsys.fdd.diskdata);
                                spcsys.fdd.buffer += ((int)spcsys.fdd.data[2] * 16 + (int)spcsys.fdd.data[3]-1) * 256;
                                spcsys.fdd.datasize = spcsys.fdd.data[0] * 256;
                                spcsys.fdd.dataidx = 0;

                            }
                            spcsys.fdd.rdVal = 0;
                            break;
                        case 0x04: // FDD Copy
                            printf("*FDD Copy\n");
                            spcsys.fdd.rSize = 7;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x05: // FDD Format
                            printf("*FDD Format\n");
                            break;
                        case 0x06: // FDD Send Status
                            printf("*FDD Send Status\n");
    #define DATA_OK 0x40
                            spcsys.fdd.rdVal = 0x80 & DATA_OK;
                            break;
                        case 0x07: // FDD Send Drive State
                            printf("*FDD Send Drive State\n");
    #define DRIVE0 0x10
                            spcsys.fdd.rdVal = 0x0f | DRIVE0;
                            break;
                        case 0x08: // FDD RAM Test
                            printf("*FDD RAM Test\n");
                            spcsys.fdd.rSize = 4;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x09: // FDD Transmit 2
                            printf("*FDD Transmit 2\n");
                            spcsys.fdd.rSize = 4;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x0A: // FDD Action
                            printf("*FDD No Action\n");
                            break;
                        case 0x0B: // FDD Transmit 1
                            printf("*FDD Transmit 1\n");
                            spcsys.fdd.rSize = 4;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x0C: // FDD Receive
                            printf("*FDD Receive\n");
                            spcsys.fdd.rSize = 4;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x0D: // FDD Go
                            printf("*FDD Go\n");
                            spcsys.fdd.rSize = 2;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x0E: // FDD Load
                            printf("*FDD Load\n");
                            spcsys.fdd.rSize = 6;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x0F: // FDD Save
                            printf("FDD Save\n");
                            spcsys.fdd.rSize = 6;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x10: // FDD Load and Go
                            printf("*FDD Load and Go\n");
                            break;

                    }
                }
                else
                {
                    if (spcsys.fdd.rSize-- > 0)
                    {
                        printf("seq=%d, data = 0x%02x\n", spcsys.fdd.seq, spcsys.fdd.wrVal);
                        spcsys.fdd.data[spcsys.fdd.seq++] = spcsys.fdd.wrVal;
                        // printf("cmd=%d\n", spcsys.fdd.cmd);
                        if (spcsys.fdd.rSize == 0)
                        {
                            int offset = ((int)spcsys.fdd.data[2] * 16 + (int)spcsys.fdd.data[3]-1) * 256;
                            //printf("Fdd Command(%d) Fired\n", spcsys.fdd.cmd);

                            if (spcsys.fdd.cmd == 0x0e)
                            {
                                printf("load(%d,%d,%d,%d),offset=%d\n", spcsys.fdd.data[0], spcsys.fdd.data[1], spcsys.fdd.data[2], spcsys.fdd.data[3], offset);
                                spcsys.fdd.buffer = (byte*)(spcsys.fdd.data[1] != 0 ? spcsys.fdd.diskdata2 : spcsys.fdd.diskdata);
                                spcsys.fdd.buffer += offset;
                                spcsys.fdd.datasize = spcsys.fdd.data[0] * 256;
                                int addr = ((unsigned short)spcsys.fdd.data[4]) * 0x100 + (unsigned short)spcsys.fdd.data[5];
                                printf("target addr=%04x, size=%d\n", addr, spcsys.fdd.datasize);
                                memcpy(&spcsys.RAM[addr], spcsys.fdd.buffer, spcsys.fdd.datasize);
                            }
                            else if (spcsys.fdd.cmd == 0x0f)
                            {
                                printf("save(%d,%d,%d,%d),offset=%d\n", spcsys.fdd.data[0], spcsys.fdd.data[1], spcsys.fdd.data[2], spcsys.fdd.data[3], offset);
                                spcsys.fdd.buffer = (byte*)(spcsys.fdd.data[1] != 0 ? spcsys.fdd.diskdata2 : spcsys.fdd.diskdata);
                                spcsys.fdd.buffer += offset;
                                spcsys.fdd.datasize = spcsys.fdd.data[0] * 256;
                                int addr = ((unsigned short)spcsys.fdd.data[4]) * 0x100 + (unsigned short)spcsys.fdd.data[5];
                                printf("source addr=%04x, size=%d\n", addr, spcsys.fdd.datasize);
                                memcpy(spcsys.fdd.buffer, &spcsys.RAM[addr], spcsys.fdd.datasize);
                            }
                        }
                    }
                }
                spcsys.fdd.PC.bits.rDAC = 1;

            }
            else if (spcsys.fdd.PC.bits.rDAC == 1) // DAV=0
            {
                //printf("FDD Ouput Data Cleared (c000 bit4 off)\n");
                spcsys.fdd.wrVal = 0;
//              printf("FDD_Read = 0h%02x (cleared)\n", spcsys.fdd.wrVal);
                spcsys.fdd.PC.bits.rDAC = 0;
            }
            if (Value & 0x20) // RFD=1
            {
//                printf("FDD Ready for Data Read (c000 bit5 on)\n");
                spcsys.fdd.PC.bits.rDAV = 1;
            }
            else if (spcsys.fdd.PC.bits.rDAV == 1) // RFD=0
            {
                //spcsys.fdd.rdVal = 0;
                //printf("FDD Input Data = 0h%02x\n", spcsys.fdd.rdVal);
                spcsys.fdd.PC.bits.rDAV = 0;
            }
            if (Value & 0x40) // DAC=1
            {
//                printf("FDD Data accepted (c000 bit6 on)\n");
            }
            if (Value & 0x80) // ATN=1
            {
//              printf("FDD Attention (c000 bit7 on)\n");
                //printf("Command = 0x%02x\n", spcsys.fdd.rdVal);
                //printf("FDD Ready for Data\n", spcsys.fdd.rdVal);
                spcsys.fdd.PC.bits.rRFD = 1;
                spcsys.fdd.isCmd = 1;
            }
//          else if (spcsys.fdd.PC.bits.rRFD == 1)
//          {
//              printf("FDD Output Data required.\n");
//              //spcsys.fdd.PC.bits.rRFD = 0;
//          }
        }
    }
}

/**
 * Read a value on Z-80 port
 * @param Port 0x0000~0xffff port address
 * @warning modified to accomodate 64K I/O address from original Marat's source
 */

 int FDD_rData;
 int FDD_wData;

byte InZ80(register word Port)
{
	if (Port >= 0x8000 && Port <= 0x8009) // Keyboard Matrix
	{
		// if (!(spcsys.cas.motor && spconf.casTurbo))
			// CheckKeyboard();
		switch (Port)
		{
		case 0x8000:
		case 0x8001:
		case 0x8002:
		case 0x8003:
		case 0x8004:
		case 0x8005:
		case 0x8006:
		case 0x8007:
		case 0x8008:
		case 0x8009:
		    if (Port == 0x8009 && spcsys.IPL_SW == 1)
            {
                spcsys.IPLK = 0;
                spcsys.IPL_SW = 0;
                return spcsys.keyMatrix[Port-0x8000] & 0xfe;
            }
            return spcsys.keyMatrix[Port-0x8000];
			break;
        case 0x9000:
        case 0x9001:
        case 0x9002:
            break;
		}
		return 0xff;
	}
	else if ((Port & 0xFFF0) == 0xc000)
    {
        if (Port == 0xc001)
        {
            if (spcsys.fdd.cmd == 0x3)
            {
                spcsys.fdd.rdVal = *(spcsys.fdd.buffer + spcsys.fdd.dataidx++);
            }
            //printf("FDD_Data > 0h%02x\n", spcsys.fdd.rdVal);
            return spcsys.fdd.rdVal;
        }
        else if (Port & 0xc002)
        {
            //printf("FDD_PC > 0h%02x\n", spcsys.fdd.PC.b);
            return spcsys.fdd.PC.b;
        }

    }
	else if ((Port & 0xE000) == 0xA000) // IPLK
	{
		spcsys.IPLK = (spcsys.IPLK)? 0 : 1;
		if (spcsys.IPLK)
            z80mem = spcsys.ROM;
        else
            z80mem = spcsys.RAM;
	}
	else if ((Port & 0xE000) == 0x2000) // GMODE
	{
		return spcsys.GMODE;
	}
	else if ((Port & 0xE000) == 0x0000) // VRAM reading
	{
		return spcsys.VRAM[Port];
	}
	else if ((Port & 0xFFFE) == 0x4000) // PSG
	{
		byte retval = 0x1f;
		if (Port & 0x01) // Data
		{
			if (spcsys.psgRegNum == 14)
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
				if (spcsys.cas.button == CAS_PLAY && spcsys.cas.motor)
				{
					if (CasRead(&spcsys.cas) == 1)
//					if (ReadVal() == 1)
						retval |= 0x80; // high
					else
						retval &= 0x7f; // low
					//printf("%d", retval & 0x80 ? 1 : 0);
					printf("c:%d\n", spcsys.cycles);
				}
				if (spcsys.cas.motor)
					retval &= (~(0x40)); // 0 indicates Motor On
				else
					retval |= 0x40;

			}
			else 
			{
				int data = RdData8910(&spcsys.ay8910);
				//printf("r(%d,%d)\n", spcsys.psgRegNum, data);
				return data;
			}
		} else if (Port & 0x02)
		{
            retval = (ReadVal() == 1 ? retval | 0x80 : retval & 0x7f);
		}
		return retval;
	}
	else if (Port == 0x4003)
	{
		return (spconf.rfp ? 1 : 0);
	}
	else if (Port == 0x4004)
	{
		byte retval = 0;
		//int pos = ftell(spconf.rfp);
		for(int i = 0; i < 8; i++) 
		{
			if (ReadVal())
				retval += 1 << (7 - i);
		}
		//fseek(spconf.rfp, pos, 0);
		ReadVal();
		//printf("%02x,", retval);
		return retval;
	}
	return 0;
}

/**
 * Writing SPC-1000 RAM
 * @param Addr 0x0000~0xffff memory address
 * @param Value a byte value to be written
 */
void WrZ80(register word Addr,register byte Value)
{
	spcsys.RAM[Addr] = Value;
//	printf("0x%04x-%02x\n", Addr, Value);
}

void PatchZ80(register Z80 *R)
{}

word LoopZ80(register Z80 *R)
{
	return INT_NONE;
}