// #include "common.h"
#include <SDL.h>
#include <SDL_keyboard.h>
#include <SDL_keycode.h>

typedef unsigned char byte;

class CKeyboard {
    private:
        typedef struct
        {
            SDL_Keycode sym;
            int keyMatIdx;
            byte keyMask;
            const char *keyName; // for debugging
        } TKeyMap;
        /**
        * Keyboard hashing table structure
        */
        typedef struct
        {
            int numEntry;
            TKeyMap *keys;
        } TKeyHashTab;

        TKeyMap spcKeyMap[70] = // the last item's keyMatIdx must be -1
        {
            { SDLK_RSHIFT, 0, 0x02, "SHIFT"},
            { SDLK_LSHIFT, 0, 0x02, "SHIFT"},
            { SDLK_LCTRL, 0, 0x04, "CTRL" },
            { SDLK_RCTRL, 0, 0x04, "CTRL" },
            { SDLK_DELETE, 0, 0x10, "BREAK" },
            { SDLK_LALT, 0, 0x40, "GRAPH" },
            { SDLK_RALT, 0, 0x40, "GRAPH"  },

            { SDLK_BACKQUOTE, 1, 0x01, "TILDE" },
            { SDLK_HOME, 1, 0x02, "HOME" },
            { SDLK_SPACE, 1, 0x04, "SPACE" },
            { SDLK_RETURN, 1, 0x08, "RETURN" },
            { SDLK_c, 1, 0x10, "C" },
            { SDLK_a, 1, 0x20, "A" },
            { SDLK_q, 1, 0x40, "Q" },
            { SDLK_1, 1, 0x80, "1" },

            { SDLK_TAB, 2, 0x01, "CAPS" },
            { SDLK_z, 2, 0x04, "Z" },
            { SDLK_RIGHTBRACKET, 2, 0x08, "]" },
            { SDLK_v, 2, 0x10, "V" },
            { SDLK_s, 2, 0x20, "S" },
            { SDLK_w, 2, 0x40, "W" },
            { SDLK_2, 2, 0x80, "2" },

            { SDLK_BACKSPACE, 3, 0x01, "DEL" },
            { SDLK_ESCAPE, 3, 0x04, "ESC" },
            { SDLK_LEFTBRACKET, 3, 0x08, "]" },
            { SDLK_b, 3, 0x10, "B" },
            { SDLK_d, 3, 0x20, "D" },
            { SDLK_e, 3, 0x40, "E" },
            { SDLK_3, 3, 0x80, "3" },

            { SDLK_RIGHT, 4, 0x04, "->" },
            { SDLK_BACKSLASH, 4, 0x08, "\\" },
            { SDLK_n, 4, 0x10, "N" },
            { SDLK_f, 4, 0x20, "F" },
            { SDLK_r, 4, 0x40, "R" },
            { SDLK_4, 4, 0x80, "4" },

            { SDLK_F1, 5, 0x02, "F1" },
            { SDLK_LEFT, 5, 0x04, "->" },
            { SDLK_m, 5, 0x10, "M" },
            { SDLK_g, 5, 0x20, "G" },
            { SDLK_t, 5, 0x40, "T" },
            { SDLK_5, 5, 0x80, "5" },

            { SDLK_F2, 6, 0x02, "F2" },
            { SDLK_EQUALS, 6, 0x04, "@" },
            { SDLK_x, 6, 0x08, "X" },
            { SDLK_COMMA, 6, 0x10, "," },
            { SDLK_h, 6, 0x20, "H" },
            { SDLK_y, 6, 0x40, "Y" },
            { SDLK_6, 6, 0x80, "6" },

            { SDLK_F3, 7, 0x02, "F3" },
            { SDLK_UP, 7, 0x04, "UP" },
            { SDLK_p, 7, 0x08, "P" },
            { SDLK_PERIOD, 7, 0x10, "." },
            { SDLK_j, 7, 0x20, "J" },
            { SDLK_u, 7, 0x40, "U" },
            { SDLK_7, 7, 0x80, "7" },

            { SDLK_F4, 8, 0x02, "F4" },
            { SDLK_DOWN, 8, 0x04, "DN" },
            { SDLK_QUOTE, 8, 0x08, ":" },
            { SDLK_SLASH, 8, 0x10, "/" },
            { SDLK_k, 8, 0x20, "K" },
            { SDLK_i, 8, 0x40, "I" },
            { SDLK_8, 8, 0x80, "8" },

            { SDLK_F5, 9, 0x02, "F5" },
            { SDLK_MINUS, 9, 0x04, "-" },
            { SDLK_0, 9, 0x08, "0" },
            { SDLK_SEMICOLON, 9, 0x10, ";" },
            { SDLK_l, 9, 0x20, "L" },
            { SDLK_o, 9, 0x40, "O" },
            { SDLK_9, 9, 0x80, "9" },

            { (SDL_Keycode)0, -1, 0, "LAST KEY" }
        };

        /**
        * Keyboard Hashing table definition.
        * initially empty.
        */
        TKeyHashTab KeyHashTab[256] = { 0, NULL };
        unsigned char keyMatrix[10];
        void BuildKeyHashTab(void);
    public:
        CKeyboard();
        unsigned char matrix(char reg) {
            // printf("%02x", keyMatrix[(reg&0xf)]);
            return keyMatrix[(reg&0xf)];
        }
        void handle_event(SDL_Event);
        void ProcessSpecialKey(SDL_Keysym ksym);
        void ProcessKeyDown(SDL_Keycode sym);
        void ProcessKeyUp(SDL_Keycode sym);
        void KeyPress(char *key);
};

