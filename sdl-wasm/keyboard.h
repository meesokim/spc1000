#include "common.h"
#include <SDL_keyboard.h>
#include <SDL_keycode.h>

void ProcessSpecialKey(SDL_Keysym ksym);
void ProcessKeyDown(SDL_Keycode sym);
void ProcessKeyUp(SDL_Keycode sym);
void BuildKeyHashTab(void);