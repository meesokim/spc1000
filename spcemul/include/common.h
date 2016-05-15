/**
 * @file common.c
 * @brief SPC-1000 emulator config structure
 * @author Kue-Hwan Sihn (ionique)
 * @date 2005~2007.1
 */

#ifndef __SPC_COMMON_H__
#define __SPC_COMMON_H__

#define MAX_PATH_LENGTH 2048

#if WIN32
#include <Windows.h>
#endif

#include "Z80.h"
#include "AY8910.h"

#include <stdio.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
typedef unsigned int UINT32;
//enum tapmodes {TAP_GUIDE, TAP_DATA, TAP_PAUSE, TAP_TRASH, TAP_STOP, TAP_STOP2, TAP_PAUSE2, TZX_PURE_TONE,
//	TZX_SEQ_PULSES, TAP_FINAL_BIT};
enum taptypes {TAP_TAP, TAP_SPC};

typedef struct
{
    // video variables
	unsigned char colorMode;
	unsigned char scanLine;
	unsigned char progressive; //interlace or progressive 576
	unsigned char dblscan;
	unsigned char frameRate;
	unsigned char zaurus_mini;
	unsigned char text_mini;
	unsigned char bw;
	unsigned char bpp;
	unsigned int  resx, resy;
	unsigned int  dispx, dispy;
	unsigned int  dx, dy;
	unsigned int  rw, rh;
	unsigned char fullscreen;

    // cassette variables
	FILE *wfp;
	FILE *rfp;
	FILE *tap_file;
	unsigned char casTurbo;
	unsigned char tape_fast_load; // 0 normal load; 1 fast load
	unsigned char rewind_on_reset;
	unsigned char tape_write; // 0 can't write; 1 can write
	unsigned char current_tap[MAX_PATH_LENGTH];
	char path_snaps[MAX_PATH_LENGTH];
	unsigned char last_selected_file[MAX_PATH_LENGTH];
	enum taptypes tape_file_type;
    // enum block_type next_block;
	unsigned char tape_stop; // 1=tape stop
	unsigned char tape_stop_fast; // 1=tape stop
    // unsigned char stop_tape_start_countdown; // 1=tape stop start countdown

    // sound variables
	unsigned char ay_emul;
	unsigned char snd_vol;

    // keyboard configuration
	unsigned char pcKeyboard;

	// model configuration
	unsigned char mode;
	unsigned char debug;
	unsigned char turbo;

	//Port variables
	unsigned char port; //SD, USB, SMB or FTP
	unsigned char smb_enable;
	unsigned char SmbUser[32];
	unsigned char SmbPwd[32];
	unsigned char SmbShare[32];
	unsigned char SmbIp[32];
	unsigned char ftp_enable;
	unsigned char FTPUser[32];
	unsigned char FTPPwd[32];
//	unsigned char FTPPath[MAX_PATH_LENGTH];
	unsigned char FTPIp[64];
	unsigned char FTPPassive;
	unsigned short FTPPort;
	char font_name[MAX_PATH_LENGTH];
	int font_size;

} SPCConfig;

/**
 * Cassette structure for tape processing, included in the SPCIO
 */
typedef struct
{
	int motor;			// Motor Status
	int pulse;			// Motor Pulse (0->1->0) causes motor state to flip
	int button;
	int rdVal;
	Uint32 lastTime;
	Uint32 cnt0, cnt1;

	int wrVal;
	Uint32 wrRisingT;	// rising time
} CassetteTape;

typedef union PC
{
    struct {
        unsigned char rDAV : 1;
        unsigned char rRFD : 1;
        unsigned char rDAC : 1;
        unsigned char rNON : 1;
        unsigned char wDAV : 1;
        unsigned char wRFD : 1;
        unsigned char wDAC : 1;
        unsigned char wATN : 1;
    } bits;
    unsigned char b;
} pc_t;

typedef struct
{
    byte rdVal;
    byte wrVal;
    byte rSize;
    byte seq;
    byte isCmd;
    byte cmd;
    byte data[6];
    pc_t PC;
    char diskfile[1024];
    char diskfile2[1024];
    char diskdata[80*16*256]; // 80 tracks 16 sectors 256 byte
    char diskdata2[80*16*256];
    byte modified;
    byte modified2;
    byte write;
    byte write2;
    byte *buffer;
    byte idx;
    int datasize;
    int dataidx;
} FloppyDisk;

typedef struct
{
    unsigned int length;
    byte bufs[1024*1024];
    byte poweron;
} Printer;

/**
 * SPC system structure, Z-80, RAM, ROM, IO(VRAM), etc
 */
typedef struct
{
	byte RAM[0x10000];	// RAM area
	byte VRAM[0x2000];	// Video Memory (6KB)
	int IPLK;				// IPLK switch for memory selection
	byte GMODE;				// GMODE (for video)
	byte psgRegNum;			// Keep PSG (AY-3-8910) register number for next turn of I/O
	Z80 Z80R;		// Z-80 register
	AY8910 ay8910;
	byte ROM[0x8000];	// ROM area
	Uint32 tick;
	Uint32 cycles;
	int refrTimer;	// timer for screen refresh
	int refrSet;	// init data for screen refresh timer
	double intrTime;	// variable for interrupt timing
	int IPL_SW;

	//int intrState;

	/* SPC-1000 keyboard matrix. Initially turned off. (high) */
	byte keyMatrix[10];
	CassetteTape cas;
    FloppyDisk fdd;
	Printer prt;
} SPCSystem;

/**
 * SPC simulation structure, realted to the timing
 */
typedef struct
{
	Uint32 baseTick, curTick, prevTick, pauseTick, menuTick;
} SPCSimul;

typedef struct
{
	SDL_Surface *emul;
	SDL_Surface *disp;
    SDL_Surface *mscr;
    SDL_Rect rect;
    int w, h;
    SDL_Color colores[16];
    TTF_Font *font;
   	unsigned int  flag;
} SDLInfo;

extern char path_snaps[MAX_PATH_LENGTH];
extern char path_taps[MAX_PATH_LENGTH];
extern char path_fdd1[MAX_PATH_LENGTH];
extern char path_fdd2[MAX_PATH_LENGTH];
extern char path_scr1[MAX_PATH_LENGTH];
extern char path_scr2[MAX_PATH_LENGTH];
extern char path_confs[MAX_PATH_LENGTH];
extern char path_tmp[MAX_PATH_LENGTH];
extern char path_disks[MAX_PATH_LENGTH];
extern char load_path_snaps[MAX_PATH_LENGTH];
extern char load_path_taps[MAX_PATH_LENGTH];
extern char load_path_scr1[MAX_PATH_LENGTH];
extern char load_path_poke[MAX_PATH_LENGTH];

extern byte DebugZ80(Z80 *R);
extern void help_menu();
extern void taps_menu();
extern void init_menu();
extern void snapshots_menu();
extern void settings_menu();

extern SPCConfig spconf;
extern SPCSystem spcsys;
extern SPCSimul  spcsim;
extern SDLInfo   spcsdl;

#define SPCCOL_OLDREV		0
#define SPCCOL_NEW1			1
#define SPCCOL_NEW2			2
#define SPCCOL_GREEN		3

#define SCANLINE_ALL		0
#define SCANLINE_NONE		1
#define SCANLINE_045_ONLY	2



#define SCREEN_WIDTH
#endif

//typedef struct {
//
//	unsigned char precision; //If set 1 emulate with more precision
////	unsigned char precision_old;
//	unsigned char npixels; //1, 2 or 4 depending on dblscan and zaurus_mini
//	unsigned char progressive; //interlace or progressive 576
////	unsigned int temporal_io;
//
//	// screen private global variables
//	SDL_Surface *screen;
//	unsigned char *screenbuffer;
//	unsigned int screen_width;
////	unsigned int translate[6144],translate2[6144];
//	unsigned char dblscan;
//	unsigned char bw;
//
//
////	int contador_flash;
//
////	unsigned int *p_translt,*p_translt2;
//	unsigned char *pixel; // current address
////	char border,flash, border_sampled;
//	int currline,currpix;
//
////	int tstados_counter; // counts tstates leaved to the next call
//	int resx,resy,bpp; // screen resolutions
////	int init_line; // cuantity to add to the base address to start to paint
////	int next_line; // cuantity to add when we reach the end of line to go to next line
////	int next_scanline; // cuantity to add to pass to the next scanline
////	int first_line; // first line to start to paint
////	int last_line; // last line to paint
////	int first_line_kb; // first line to start to paint the keyboard
////	int last_line_kb; // last line to paint the keyboard
////	int first_pixel; // first pixel of a line to paint
////	int last_pixel; // last pixel of a line to paint
////	int next_pixel; // next pixel
//	int pixancho,pixalto; // maximum pixel data for width and height
//	int jump_pixel;
//	int upper_border_line; //63 or 62 for 48k or 128k
//	int lower_border_line; //upper_border_line + 192
//	int start_screen; //Pixel at which the interrupt is generated
//	int cpufreq; //frequency CPU
////	int tstatodos_frame; //number of tstados per frame
//	int pixels_octect; //2 bits in the octect
//	int pixels_word; //2 bits in the word
//	int start_contention; //start tstados for contention
//	//int end_contention; //end tstados for contention
//
//	//unsigned char screen_snow; // 0-> no emulate snow; 1-> emulate snow
////	unsigned char fetch_state;
////	unsigned char contended_zone; // 0-> no contention; 1-> contention possible
////	int cicles_counter; // counts how many pixel clock cicles passed since las interrupt
//
////	char ulaplus; // 0 = inactive; 1 = active
////	unsigned char ulaplus_reg; // contains the last selected register in the ULAPlus
////	unsigned char ulaplus_palete[64]; // contains the current palete
//
//	// keyboard private global variables
//
////	unsigned char s8,s9,s10,s11,s12,s13,s14,s15;
////	unsigned char k8,k9,k10,k11,k12,k13,k14,k15;
////	unsigned char readed;
////	//unsigned char tab_extended;
////	unsigned char esc_again;
//
//	// kempston joystick private global variables
//
////	unsigned char js,jk;
//
//	// Linux joystick private global variables
//
//	//unsigned char use_js;
//	//unsigned char updown,leftright;
//
//	// sound global variables
//
////	int tst_sample; // number of tstates per sample
////	int freq; // frequency for reproduction
////	int format; // 0: 8 bits, 1: 16 bits LSB, 2: 16 bits MSB
////	signed char sign; // 0: unsigned; -128: signed
////	int channels; // number of channels
////	int buffer_len; // sound buffer length (in samples)
////	int increment; // quantity to add to jump to the next sample
//	unsigned char volume; // volume
////	unsigned char sample1[4]; // buffer with precalculated sample 1 (for buzzer) -currently not used
////	unsigned char sample1b[4]; // buffer with prec. sample 1 (for AY-3-8912) -currently not used
////	//unsigned char sample0[4]; // buffer with precalculated sample 0
////	unsigned char sound_bit;
////	unsigned char sound_bit_mic;
////	unsigned int tstados_counter_sound;
////	unsigned int low_filter;
////	unsigned int *current_buffer;
////	unsigned char num_buff;
////	unsigned int sound_cuantity; // counter for the buffer
////	unsigned char ay_registers[16]; // registers for the AY emulation
////	unsigned int aych_a,aych_b,aych_c,aych_n,aych_envel; // counters for AY emulation
////	unsigned char ayval_a,ayval_b,ayval_c,ayval_n;
//	unsigned char ay_emul; // 0: no AY emulation; 1: AY emulation
////	unsigned char audio_mode; //mono, ABC, ACB, BAC
////	unsigned int vol_a,vol_b,vol_c;
////	unsigned int tst_ay;
////	unsigned int ay_latch;
////	signed char ay_envel_data;
////	unsigned char ay_envel_way;
//	//unsigned char sound_current_data;
//
//	//Z80 instruction variables
////	unsigned int wr;
////	unsigned int r_fetch;
////	unsigned int io;
////	unsigned int contention;
//
//	// bus global variables
//
////	unsigned char bus_counter;
////	unsigned char bus_data;
//	unsigned char issue; // 2= 48K issue 2, 3= 48K issue 3
//	unsigned char mode; // 0=48K, 1=128K, 2=+2, 3=+3 4=sp
////	unsigned char videosystem; //0=PAL, 1=NTSC
//	unsigned char joystick[2]; // 0=cursor, 1=kempston, 2=sinclair1, 3=sinclair2
////	unsigned char port254;
//
//
//	// tape global variables
//
//	enum tapmodes tape_current_mode;
////	enum block_type next_block;
//	unsigned char tape_stop; // 1=tape stop
//	unsigned char tape_stop_fast; // 1=tape stop
////	unsigned char stop_tape_start_countdown; // 1=tape stop start countdown
//	enum taptypes tape_file_type;
////	unsigned int tape_counter0;
////	unsigned int tape_counter1;
////	unsigned int tape_counter_rep;
////	unsigned char tape_byte;
////	unsigned char tape_bit;
////	unsigned char tape_readed;
////	unsigned int tape_byte_counter;
////	unsigned int tape_pause_at_end;
////	unsigned int tape_position;
//	FILE *tap_file;
//	unsigned char tape_fast_load; // 0 normal load; 1 fast load
//	unsigned char rewind_on_reset;
////	unsigned char pause_instant_load;
//	unsigned char current_tap[MAX_PATH_LENGTH];
//	unsigned char last_selected_file[MAX_PATH_LENGTH];
////	unsigned char last_selected_poke_file[MAX_PATH_LENGTH];
////
////	unsigned char tape_current_bit;
////	unsigned int tape_block_level;
////	unsigned int tape_sync_level0;
////	unsigned int tape_sync_level1;
////	unsigned int tape_bit0_level;
////	unsigned int tape_bit1_level;
////	unsigned char tape_bits_at_end;
////	unsigned int tape_loop_counter;
//	unsigned int tape_start_countdwn;
////	unsigned int pause_fastload_countdwn;
////	long tape_loop_pos;
////
//	unsigned char tape_write; // 0 can't write; 1 can write
//
//	// Microdrive global variables
////	FILE *mdr_file;                  // Current microdrive file
////	unsigned char mdr_current_mdr[MAX_PATH_LENGTH]; // current path and name for microdrive file
////	unsigned char mdr_active;	// 0: not installed; 1: installed
////	unsigned char mdr_paged;	// 0: not pagined; 1: pagined
////	unsigned int mdr_tapehead; // current position in the tape
////	unsigned int mdr_bytes;      // number of bytes read or written in this transfer
////	unsigned int mdr_maxbytes; // maximum number of bytes to read or write in this transfer
////	unsigned int mdr_gap;         // TSTATEs remaining for GAP end
////	unsigned int mdr_nogap;      // TSTATEs remaining for next GAP
////	unsigned char mdr_cartridge[137923]; // current cartridge
////	unsigned char mdr_drive; // current drive
////	byte mdr_old_STATUS; // to detect an edge in COM CLK
////	unsigned char mdr_modified; // if a sector is stored, this change to know that it must be stored in the file
//
//	// OSD global variables
//
//	unsigned char osd_text[200];
//	unsigned int osd_time;
//
//	// pagination global variables
//
//	unsigned char mport1,mport2; // ports for memory management (128K and +3)
//	unsigned int video_offset; // 0 for page 5, and 32768 for page 7
////	unsigned char *block0,*block1,*block2,*block3; // pointers for memory access (one for each 16K block).
//
//	// public
//
////	unsigned char memoria[196608]; // memory (12 pages of 16K each one). 4 for ROM, and 8 for RAM
////	unsigned char shadowrom[8192]; // space for Interface I's ROMs
////	unsigned char interr;
////	unsigned char readkeyboard;
////	unsigned char mustlock;
//	//unsigned char other_ret; // 0=no change; 1=memory returns RET (201)
//
//	unsigned char turbo;
//	unsigned char turbo_state;
////	unsigned int keyboard_buffer[2][KB_BUFFER_LENGHT];
////	unsigned int kbd_buffer_pointer;
//	unsigned char *key;
//	unsigned char joystick_number;
//	SDL_Joystick *joystick_sdl[2];
//	unsigned char joy_axis_x_state[2];
//	unsigned char joy_axis_y_state[2];
//	unsigned int joybuttonkey[2][23];
//	unsigned char joypad_as_joystick[2];
//	unsigned char rumble[2];
//	unsigned char vk_auto;
//	unsigned char vk_rumble;
//	unsigned char vk_is_active;
//	unsigned char autoconf;
//	unsigned char ignore_z80_joy_conf;
//
//
//
//} computer;


