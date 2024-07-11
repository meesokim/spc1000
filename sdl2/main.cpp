#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <SDL.h>
//#include <SDL_mixer.h>
#include "cpu.h"
#include "mc6847.h"
#include "ay8910.h"
#include "keyboard.h"
#include "cassette.h"
#include "spcall.h"
// #include "../kernel/platform.h"
// #include "../kernel/wiring.h"

#define FONT                    vgafont8
#define BIT_SHIFT               (7 - s_bit_no)

#define CHAR_W                  8
#define CHAR_H                  8

static unsigned char vgafont8[128 * 8]= {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7e, 0x81, 0xa5, 0x81, 0xbd, 0x99, 0x81, 0x7e,
    0x7e, 0xff, 0xdb, 0xff, 0xc3, 0xe7, 0xff, 0x7e,
    0x6c, 0xfe, 0xfe, 0xfe, 0x7c, 0x38, 0x10, 0x00,
    0x10, 0x38, 0x7c, 0xfe, 0x7c, 0x38, 0x10, 0x00,
    0x38, 0x7c, 0x38, 0xfe, 0xfe, 0x7c, 0x38, 0x7c,
    0x10, 0x10, 0x38, 0x7c, 0xfe, 0x7c, 0x38, 0x7c,
    0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00,
    0xff, 0xff, 0xe7, 0xc3, 0xc3, 0xe7, 0xff, 0xff,
    0x00, 0x3c, 0x66, 0x42, 0x42, 0x66, 0x3c, 0x00,
    0xff, 0xc3, 0x99, 0xbd, 0xbd, 0x99, 0xc3, 0xff,
    0x0f, 0x07, 0x0f, 0x7d, 0xcc, 0xcc, 0xcc, 0x78,
    0x3c, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x7e, 0x18,
    0x3f, 0x33, 0x3f, 0x30, 0x30, 0x70, 0xf0, 0xe0,
    0x7f, 0x63, 0x7f, 0x63, 0x63, 0x67, 0xe6, 0xc0,
    0x99, 0x5a, 0x3c, 0xe7, 0xe7, 0x3c, 0x5a, 0x99,
    0x80, 0xe0, 0xf8, 0xfe, 0xf8, 0xe0, 0x80, 0x00,
    0x02, 0x0e, 0x3e, 0xfe, 0x3e, 0x0e, 0x02, 0x00,
    0x18, 0x3c, 0x7e, 0x18, 0x18, 0x7e, 0x3c, 0x18,
    0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00,
    0x7f, 0xdb, 0xdb, 0x7b, 0x1b, 0x1b, 0x1b, 0x00,
    0x3e, 0x63, 0x38, 0x6c, 0x6c, 0x38, 0xcc, 0x78,
    0x00, 0x00, 0x00, 0x00, 0x7e, 0x7e, 0x7e, 0x00,
    0x18, 0x3c, 0x7e, 0x18, 0x7e, 0x3c, 0x18, 0xff,
    0x18, 0x3c, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x00,
    0x18, 0x18, 0x18, 0x18, 0x7e, 0x3c, 0x18, 0x00,
    0x00, 0x18, 0x0c, 0xfe, 0x0c, 0x18, 0x00, 0x00,
    0x00, 0x30, 0x60, 0xfe, 0x60, 0x30, 0x00, 0x00,
    0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xfe, 0x00, 0x00,
    0x00, 0x24, 0x66, 0xff, 0x66, 0x24, 0x00, 0x00,
    0x00, 0x18, 0x3c, 0x7e, 0xff, 0xff, 0x00, 0x00,
    0x00, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x30, 0x78, 0x78, 0x30, 0x30, 0x00, 0x30, 0x00,
    0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x6c, 0x6c, 0xfe, 0x6c, 0xfe, 0x6c, 0x6c, 0x00,
    0x30, 0x7c, 0xc0, 0x78, 0x0c, 0xf8, 0x30, 0x00,
    0x00, 0xc6, 0xcc, 0x18, 0x30, 0x66, 0xc6, 0x00,
    0x38, 0x6c, 0x38, 0x76, 0xdc, 0xcc, 0x76, 0x00,
    0x60, 0x60, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00,
    0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00,
    0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00,
    0x00, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x60,
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00,
    0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x80, 0x00,
    0x7c, 0xc6, 0xce, 0xde, 0xf6, 0xe6, 0x7c, 0x00,
    0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x00,
    0x78, 0xcc, 0x0c, 0x38, 0x60, 0xcc, 0xfc, 0x00,
    0x78, 0xcc, 0x0c, 0x38, 0x0c, 0xcc, 0x78, 0x00,
    0x1c, 0x3c, 0x6c, 0xcc, 0xfe, 0x0c, 0x1e, 0x00,
    0xfc, 0xc0, 0xf8, 0x0c, 0x0c, 0xcc, 0x78, 0x00,
    0x38, 0x60, 0xc0, 0xf8, 0xcc, 0xcc, 0x78, 0x00,
    0xfc, 0xcc, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x00,
    0x78, 0xcc, 0xcc, 0x78, 0xcc, 0xcc, 0x78, 0x00,
    0x78, 0xcc, 0xcc, 0x7c, 0x0c, 0x18, 0x70, 0x00,
    0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x00,
    0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x60,
    0x18, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x18, 0x00,
    0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00,
    0x60, 0x30, 0x18, 0x0c, 0x18, 0x30, 0x60, 0x00,
    0x78, 0xcc, 0x0c, 0x18, 0x30, 0x00, 0x30, 0x00,
    0x7c, 0xc6, 0xde, 0xde, 0xde, 0xc0, 0x78, 0x00,
    0x30, 0x78, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0x00,
    0xfc, 0x66, 0x66, 0x7c, 0x66, 0x66, 0xfc, 0x00,
    0x3c, 0x66, 0xc0, 0xc0, 0xc0, 0x66, 0x3c, 0x00,
    0xf8, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0xf8, 0x00,
    0xfe, 0x62, 0x68, 0x78, 0x68, 0x62, 0xfe, 0x00,
    0xfe, 0x62, 0x68, 0x78, 0x68, 0x60, 0xf0, 0x00,
    0x3c, 0x66, 0xc0, 0xc0, 0xce, 0x66, 0x3e, 0x00,
    0xcc, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0xcc, 0x00,
    0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
    0x1e, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78, 0x00,
    0xe6, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0xe6, 0x00,
    0xf0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xfe, 0x00,
    0xc6, 0xee, 0xfe, 0xfe, 0xd6, 0xc6, 0xc6, 0x00,
    0xc6, 0xe6, 0xf6, 0xde, 0xce, 0xc6, 0xc6, 0x00,
    0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00,
    0xfc, 0x66, 0x66, 0x7c, 0x60, 0x60, 0xf0, 0x00,
    0x78, 0xcc, 0xcc, 0xcc, 0xdc, 0x78, 0x1c, 0x00,
    0xfc, 0x66, 0x66, 0x7c, 0x6c, 0x66, 0xe6, 0x00,
    0x78, 0xcc, 0xe0, 0x70, 0x1c, 0xcc, 0x78, 0x00,
    0xfc, 0xb4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xfc, 0x00,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00,
    0xc6, 0xc6, 0xc6, 0xd6, 0xfe, 0xee, 0xc6, 0x00,
    0xc6, 0xc6, 0x6c, 0x38, 0x38, 0x6c, 0xc6, 0x00,
    0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x30, 0x78, 0x00,
    0xfe, 0xc6, 0x8c, 0x18, 0x32, 0x66, 0xfe, 0x00,
    0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00,
    0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x02, 0x00,
    0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00,
    0x10, 0x38, 0x6c, 0xc6, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00,
    0xe0, 0x60, 0x60, 0x7c, 0x66, 0x66, 0xdc, 0x00,
    0x00, 0x00, 0x78, 0xcc, 0xc0, 0xcc, 0x78, 0x00,
    0x1c, 0x0c, 0x0c, 0x7c, 0xcc, 0xcc, 0x76, 0x00,
    0x00, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00,
    0x38, 0x6c, 0x60, 0xf0, 0x60, 0x60, 0xf0, 0x00,
    0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8,
    0xe0, 0x60, 0x6c, 0x76, 0x66, 0x66, 0xe6, 0x00,
    0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00,
    0x0c, 0x00, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78,
    0xe0, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0xe6, 0x00,
    0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
    0x00, 0x00, 0xcc, 0xfe, 0xfe, 0xd6, 0xc6, 0x00,
    0x00, 0x00, 0xf8, 0xcc, 0xcc, 0xcc, 0xcc, 0x00,
    0x00, 0x00, 0x78, 0xcc, 0xcc, 0xcc, 0x78, 0x00,
    0x00, 0x00, 0xdc, 0x66, 0x66, 0x7c, 0x60, 0xf0,
    0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0x1e,
    0x00, 0x00, 0xdc, 0x76, 0x66, 0x60, 0xf0, 0x00,
    0x00, 0x00, 0x7c, 0xc0, 0x78, 0x0c, 0xf8, 0x00,
    0x10, 0x30, 0x7c, 0x30, 0x30, 0x34, 0x18, 0x00,
    0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x76, 0x00,
    0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00,
    0x00, 0x00, 0xc6, 0xd6, 0xfe, 0xfe, 0x6c, 0x00,
    0x00, 0x00, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0x00,
    0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8,
    0x00, 0x00, 0xfc, 0x98, 0x30, 0x64, 0xfc, 0x00,
    0x1c, 0x30, 0x30, 0xe0, 0x30, 0x30, 0x1c, 0x00,
    0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00,
    0xe0, 0x30, 0x30, 0x1c, 0x30, 0x30, 0xe0, 0x00,
    0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x38, 0x6c, 0xc6, 0xc6, 0xfe, 0x00,
};

static unsigned char keyNormal_it[] = {
    0x0, 0x0, 0x0, 0x0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
    'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', '0', '\r', 0x0, '\b', '\t', ' ', '\'', 0x0, 0x0,
    '+', '<', 0x0, 0x0, 0x0, '\\', ',', '.', '-', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, '/', '*', '-', '+', '\r', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '0', '.', '<', 0x0, 0x0, '='
};

static unsigned char keyShift_it[] = {
    0x0, 0x0, 0x0, 0x0, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '"',
    0x0, '$', '%', '&', '/', '(', ')', '=', '\r', 0x0, '\b', '\t', ' ', '?', '^', 0x0,
    '*', '>', 0x0, 0x0, 0x0, '|', ';', ':', '_', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, '/', '*', '-', '+', '\r', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '0', '.', '>', 0x0, 0x0, '='
};

#if defined(__cplusplus)
extern "C" {
#endif

// __attribute__ ((interrupt ("IRQ"))) void interrupt_irq() {
//     SDL_Interrupt_Handler();
// }

#if defined(__cplusplus)
}
#endif

struct _screen {
    unsigned char c;
    struct {
        unsigned char r;
        unsigned char g;
        unsigned char b;
    } fore;
    struct {
        unsigned char r;
        unsigned char g;
        unsigned char b;
    } back;
};

#define txt_width  40
#define txt_height 25

static int cur_x;
static int cur_y;
static int cursor_visible;
static struct _screen screen[txt_width * txt_height];
static SDL_Rect crect;

void SDL_DrawString(const char *s) {
    char c;

    while (*s) {
        c = *s++;
        if (c == '\r') {
            cur_x = 0;
        }
        else if (c == '\n') {
            cur_y++;
            if (cur_y >= txt_height) {
                memcpy(&screen[0], &screen[txt_width], sizeof(screen) - sizeof(struct _screen) * txt_width);
                for (int i = txt_width * (txt_height - 1); i < txt_width * txt_height; i++) {
                    screen[i].c = ' ';
                    screen[i].fore.r = 255;
                    screen[i].fore.g = 255;
                    screen[i].fore.b = 255;
                    screen[i].back.r = 98;
                    screen[i].back.g = 0;
                    screen[i].back.b = 32;
                }
                cur_y--;
            }
        }
        else {
            screen[cur_y * txt_width + cur_x].c = c;
            cur_x++;
            if (cur_x >= txt_width) {
                cur_x = 0;
                cur_y++;
                if (cur_y >= txt_height) {
                    memcpy(&screen[0], &screen[txt_width], sizeof(screen) - sizeof(struct _screen) * txt_width);
                    for (int i = txt_width * (txt_height - 1); i < txt_width * txt_height; i++) {
                        screen[i].c = ' ';
                        screen[i].fore.r = 255;
                        screen[i].fore.g = 255;
                        screen[i].fore.b = 255;
                        screen[i].back.r = 98;
                        screen[i].back.g = 0;
                        screen[i].back.b = 32;
                    }
                    cur_y--;
                }
            }
        }
    }
    cursor_visible = 1;
}

void SDL_DrawStringAt(int y, int x, const char *s) {
    cur_x = x % txt_width;
    cur_y = y % txt_height;
    SDL_DrawString(s);
}

void SDL_DrawChar(char c) {
    if (c == '\r') {
        cur_x = 0;
    }
    else if (c == '\n') {
        cur_y++;
        if (cur_y >= txt_height) {
            memcpy(&screen[0], &screen[txt_width], sizeof(screen) - sizeof(struct _screen) * txt_width);
            for (int i = txt_width * (txt_height - 1); i < txt_width * txt_height; i++) {
                screen[i].c = ' ';
                screen[i].fore.r = 255;
                screen[i].fore.g = 255;
                screen[i].fore.b = 255;
                screen[i].back.r = 98;
                screen[i].back.g = 0;
                screen[i].back.b = 32;
            }
            cur_y--;
        }
    }
    else {
        screen[cur_y * txt_width + cur_x].c = c;
        cur_x++;
        if (cur_x >= txt_width) {
            cur_x = 0;
            cur_y++;
            if (cur_y >= txt_height) {
                memcpy(&screen[0], &screen[txt_width], sizeof(screen) - sizeof(struct _screen) * txt_width);
                for (int i = txt_width * (txt_height - 1); i < txt_width * txt_height; i++) {
                    screen[i].c = ' ';
                    screen[i].fore.r = 255;
                    screen[i].fore.g = 255;
                    screen[i].fore.b = 255;
                    screen[i].back.r = 98;
                    screen[i].back.g = 0;
                    screen[i].back.b = 32;
                }
                cur_y--;
            }
        }
    }
    cursor_visible = 1;
}

void SDL_DrawCharAt(int y, int x, char c) {
    cur_x = x % txt_width;
    cur_y = y % txt_height;
    SDL_DrawChar(c);
}

void SDL_InitConsole(int w, int h) {
    crect.x = (w - txt_width * CHAR_W) / 2;
    crect.y = (h - txt_height * CHAR_H) / 2;
    crect.w = txt_width * CHAR_W;
    crect.h = txt_height * CHAR_H;

    for (int i = 0; i < txt_width * txt_height; i++) {
        screen[i].c = ' ';
        screen[i].fore.r = 255;
        screen[i].fore.g = 255;
        screen[i].fore.b = 255;
        screen[i].back.r = 98;
        screen[i].back.g = 0;
        screen[i].back.b = 32;
    }

    cur_x = 0;
    cur_y = 0;
}

void SDL_RenderConsole(SDL_Renderer *renderer) {
    int x, y, c_x;
    int index = 0;

    for (y = crect.y; index < txt_width * txt_height; y += CHAR_H) {
        for (c_x = 0, x = crect.x; c_x < txt_width && index < txt_width * txt_height; index++, c_x++, x += CHAR_W) {
            int s_offset = (int) screen[index].c * CHAR_W * CHAR_H;
            for (int f_y = 0; f_y < CHAR_H; f_y++) {
                for (int f_x = 0; f_x < CHAR_W; f_x++) {
                    int s_byte_no = s_offset / 8;
                    int s_bit_no = s_offset % 8;

                    unsigned char s_byte = FONT[s_byte_no];
                    if ((s_byte >> BIT_SHIFT) & 0x1)
                        SDL_SetRenderDrawColor(renderer, screen[index].fore.r, screen[index].fore.g, screen[index].fore.b, 255);
                    else
                        SDL_SetRenderDrawColor(renderer, screen[index].back.r, screen[index].back.g, screen[index].back.b, 255);
                    SDL_RenderDrawPoint(renderer, x + f_x, y + f_y);
                    s_offset++;
                }
            }
        }
    }

    if (cursor_visible) {
        SDL_Rect rect;
        rect.x = crect.x + cur_x * CHAR_W;
        rect.y = crect.y + cur_y * CHAR_H;
        rect.w = CHAR_W;
        rect.h = CHAR_H;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
}

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

SDL_Palette *create_palette()
{
    SDL_Palette *p = SDL_AllocPalette(2);
    p->colors[0].r = 0xff;
    p->colors[0].g = 0x00;
    p->colors[0].b = 0x00;
    p->colors[1].r = 0xff;
    p->colors[1].g = 0xff;
    p->colors[1].b = 0xff;
    return p;
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
					if (cassette.read(cpu.getCycles(), rb(0, 0x3c5)) == 1)
							retval |= 0x80; // high
						else
							retval &= 0x7f; // low
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
                    cpu.set_turbo(cassette.motor);
                    wb(NULL, 0x3c5, cassette.motor ? 1 : 90);
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
#define SPC1000_AUDIO_BUFFER_SIZE 1024

static unsigned ptime;
static unsigned etime;
SDL_AudioSpec audioSpec;
int audid;

void callback(
  void* userdata,
  Uint8* stream,
  int    len)
{
    int samples = len / (sizeof(int16_t) * audioSpec.channels);
    ay8910.pushbuf((int16_t*)stream, samples);
    // for(int i = 0; i < samples; i++)
    // {
    //     printf("%02x", stream[i]);
    // }
    // SDL_memset(stream, 0, len);
    // printf("samples:%d\n", len);
    // renderAudioDevice(hbc56Device(i), str, samples);
}

unsigned int execute(Uint32 interval, void* name)
{
    static int frame = 0;
    etime = SDL_GetTicks();
    // int steps = (etime - ptime) * CPU_FREQ/1000;
    cpu.pulse_irq(0);
    int steps = cpu.exec(etime);
    cpu.clr_irq();
    if (frame++%2)
        mc6847.Update();
    ay8910.update(etime);
    ptime = etime;
    return 0;
}

#include <iostream>

SDL_Renderer *renderer;
SDL_Surface *surface;
SDL_Texture *texture;
SDL_Event event;
int w, h;

void  main_loop()
{
    SDL_Delay(16);
    execute(16, NULL);
    // int step = (ctime - ptime) * CPU_FREQ/1000;
    // if (!step)
    //     continue;
    // ptime = ctime;
    // // printf("pc=%04x %d\n", cpu.r->pc, step);
    // cpu.step_n(step);
    // printf("pc=%04x %d\n", cpu.r->pc, step);
    // cpu.debug();
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            break;
        } else {
            kbd.handle_event(event);
        }
    }
    // if (compare_timer(&tw)) {
    //     led_status = led_status == LOW ? HIGH : LOW;
    //     digitalWrite(16, led_status);
    //     cursor_visible = cursor_visible ? 0 : 1;
    // }
    // for(uint16_t* p = pixels; p != &pixels[w * h] ; p+=2) {
    //     p[0] = 0xff00; p[1] = 0x0;
    // }
    // uint16_t *pixels = ;
    // SDL_LockTexture(texture);
    int ret = SDL_UpdateTexture(texture, NULL, mc6847.GetBuffer(), w*2);
    // if (ret < 0) 
    // {
    //     printf("%s\n", SDL_GetError());
    //     exit(0);
    // }
    // SDL_UnlockTexture(texture);
    // SDL_SetRenderDrawColor(renderer, 213, 41, 82, 255);
    // SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderCopy(renderer, SDL_CreateTextureFromSurface(renderer, surface), NULL, NULL);
    // SDL_BlitScaled(surface, NULL, fb, NULL);
    // SDL_UpdateWindowSurface(screen);
    // SDL_RenderConsole(renderer);

    SDL_RenderPresent(renderer);
    // SDL_MixAudio(ay9810.)
}


int main() {
    SDL_Window *screen;
    // struct timer_wait tw;
    int led_status = LOW;
    cpu.init();
    cpu.set_read_write(rb, wb);
    cpu.set_in_out(in, out);
    mc6847.Initialize();

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
    // Default screen resolution (set in config.txt or auto-detected)
    // SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &screen, &renderer);
    w = 320; h = 240;
    SDL_CreateWindowAndRenderer(w * 2, h * 2, SDL_WINDOW_BORDERLESS, &screen, &renderer);
    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w * 2, h * 2, 32, 0, 0, 0, 0);
    SDL_SetColorKey(surface, SDL_TRUE, 0x0);
    SDL_LockSurface(surface);
    // for(int i = 0; i < w * 2 * 20 * 2; i++)
    //     ((char *)surface->pixels)[i*4] = 0xff;
    SDL_UnlockSurface(surface);
    // surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 16, SDL_PIXELFORMAT_RGB565);
    // palette = create_palette();
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, w, h);
    if (!texture) {
        printf("%s\n", SDL_GetError());
    }
    //Initialize SDL2 Audio
    SDL_AudioSpec specs = {};
    specs.freq = SPC1000_AUDIO_FREQ;
    specs.format = AUDIO_S16SYS;
    specs.channels = 1;
    specs.samples = SPC1000_AUDIO_BUFFER_SIZE;
    specs.callback = callback;
    constexpr int PLAYBACK_DEV = 0;
    audid  = SDL_OpenAudioDevice( nullptr, PLAYBACK_DEV, &specs, &audioSpec, SDL_AUDIO_ALLOW_CHANNELS_CHANGE );
    if( audid == 0 )
    {
        std::cerr << "Error opening audio device: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_PauseAudioDevice(audid, 0);
    // SDL_Surface *fb = SDL_CreateRGBSurfaceWithFormat(0, w, h, 8, SDL_PIXELFORMAT_INDEX8);
    // // Sets a specific screen resolution
    // // SDL_CreateWindowAndRenderer(32 + 320 + 32, 32 + 200 + 32, SDL_WINDOW_FULLSCREEN, &screen, &renderer);
    // SDL_Color colors[2] = {{255,0,0,255}, {0,255,0,255}};
    // SDL_SetPaletteColors(surface->format->palette, colors, 0, 2);
    // SDL_SetSurfacePalette(fb, palette);
    // SDL_SetSurfacePalette(surface, palette);
    // SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
    // SDL_SetSurfaceBlendMode(fb, SDL_BLENDMODE_NONE);
    // SDL_GetWindowSize(screen, &w, &h);
    // SDL_InitConsole(w, h);

    // SDL_DrawStringAt(1, (txt_width - 22) / 2, "**** RASPBERRY-PI ****");
    // SDL_DrawStringAt(3, (txt_width - 30) / 2, "BARE-METAL SDL SYSTEM TEMPLATE\r\n");

    pinMode(16, OUTPUT);
    // register_timer(&tw, 250000);
    ptime = SDL_GetTicks();
    ay8910.initTick(ptime);
    cpu.initTick(ptime);
    cassette.loaddir("../tape");
    cassette.load("../tape/demo.tap");
    // SDL_TimerID timerID = SDL_AddTimer(16, execute, (void *)"SDL");
#ifdef EMSCRIPTEN    
    emscripten_set_main_loop(main_loop, -1, 1);
#else
    do {
    main_loop();
    //     SDL_Delay(16);
    //     execute(16, NULL);
    //     // int step = (ctime - ptime) * CPU_FREQ/1000;
    //     // if (!step)
    //     //     continue;
    //     // ptime = ctime;
    //     // // printf("pc=%04x %d\n", cpu.r->pc, step);
    //     // cpu.step_n(step);
    //     // printf("pc=%04x %d\n", cpu.r->pc, step);
    //     // cpu.debug();
    //     while (SDL_PollEvent(&event)) {
    //         if (event.type == SDL_QUIT) {
    //             break;
    //         } else {
    //             kbd.handle_event(event);
    //         }
    //     }
    //     // if (compare_timer(&tw)) {
    //     //     led_status = led_status == LOW ? HIGH : LOW;
    //     //     digitalWrite(16, led_status);
    //     //     cursor_visible = cursor_visible ? 0 : 1;
    //     // }
    //     // for(uint16_t* p = pixels; p != &pixels[w * h] ; p+=2) {
    //     //     p[0] = 0xff00; p[1] = 0x0;
    //     // }
    //     // uint16_t *pixels = ;
    //     // SDL_LockTexture(texture);
    //     int ret = SDL_UpdateTexture(texture, NULL, mc6847.GetBuffer(), w*2);
    //     // if (ret < 0) 
    //     // {
    //     //     printf("%s\n", SDL_GetError());
    //     //     exit(0);
    //     // }
    //     // SDL_UnlockTexture(texture);
    //     // SDL_SetRenderDrawColor(renderer, 213, 41, 82, 255);
    //     // SDL_RenderClear(renderer);
    //     SDL_RenderCopy(renderer, texture, NULL, NULL);
    //     // SDL_BlitScaled(surface, NULL, fb, NULL);
    //     // SDL_UpdateWindowSurface(screen);
    //     // SDL_RenderConsole(renderer);

    //     SDL_RenderPresent(renderer);
    //     // SDL_MixAudio(ay9810.)
    }
    while(event.type != SDL_QUIT);
#endif
}
