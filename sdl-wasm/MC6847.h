#ifndef __MC6847_H__
#define __MC6847_H__

#include <SDL.h>
#include <SDL_image.h>

#include <stdlib.h>
#include <memory.h>

#define SET_TEXTPAGE	1
#define SET_GRAPHIC		2
#define SET_TEXTMODE	3

#define _CLR0 {0x00, 0x00, 0x00} /* BLACK */
#define	_CLR1 {0x07, 0xff, 0x00} /* GREEN */ 
#define	_CLR2 {0xff, 0xff, 0x00} /* YELLOW */
#define	_CLR3 {0x3b, 0x08, 0xff} /* BLUE */
#define	_CLR4 {0xcc, 0x00, 0x3b} /* RED */ 
#define	_CLR5 {0xff, 0xff, 0xff} /* BUFF */
#define	_CLR6 {0x07, 0xe3, 0x99} /* CYAN */ 
#define	_CLR7 {0xff, 0x1c, 0xff} /* MAGENTA */ 
#define	_CLR8 {0xff, 0x81, 0x00} /* ORANGE */

#define	_CLR9 {0x07, 0xff, 0x00} /* GREEN */
#define	_CLR10 {0xff, 0xff, 0xff} /* BUFF */

#define	_CLR11 {0x00, 0x44, 0x00} /* ALPHANUMERIC DARK GREEN */ 
#define	_CLR12 {0x07, 0xff, 0x00} /* ALPHANUMERIC BRIGHT GREEN */  
#define	_CLR13 {0x91, 0x00, 0x00} /* ALPHANUMERIC DARK ORANGE */
#define	_CLR14 {0xff, 0x81, 0x00} /* ALPHANUMERIC BRIGHT ORANGE */

typedef unsigned char PIXEL;

PIXEL * InitMC6847(void);
void Update9918(Uint8 gmode, Uint8 *, Uint8 *, Uint8 *);
void Update6847(Uint8 gmode, Uint8 *VRAM, PIXEL *fb);
void CloseMC6847(void);

#endif // __MC6847_H__
