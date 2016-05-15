/*
 * Copyright 2014 (C) Miso Kim
 * Copyright (C) 2012 Fabio Olimpieri
 * Copyright 2003-2009 (C) Raster Software Vigo (Sergio Costas)
 * This file is modified from a part of FBZX Wii
 *
 * FBSPC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * FBSPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "common.h"
#include "menus.h"

extern int SaveImageFile(char *szFile);
extern int LoadImageFile(char *szFile);

void clean_screen();

void print_string(SDL_Surface *menu, char *cadena, int x, int y, unsigned char color, unsigned char back);

void flip_screen(SDL_Surface *menu) {
	SDL_Rect rect;
	rect.x = rect.y = 0;
	rect.w = spcsdl.w*2; rect.h = spcsdl.h*2;
    SDL_SoftStretch(menu, NULL, spcsdl.emul, &rect);		
//    SDL_BlitSurface(menu, NULL, spcsdl.emul, NULL);
    SDL_Flip(spcsdl.emul);
    return;
}

// shows the settings menu

void print_copy(SDL_Surface *menu) {

	print_string(menu,"(C)2014 8bit micom club @ NAVER",-1,455,13,0);

}

// prints the string CADENA in X,Y (centered if X=-1), with colors COLOR and BACK

//SDL_Color translate_color(Uint32 int_color) {
//    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
//        SDL_Color color={(int_color & 0x00ff0000)/0x10000,(int_color &
//0x0000ff00)/0x100,(int_color & 0x000000ff),0};
//    #else
//        SDL_Color color={(int_color & 0x000000ff),(int_color &
//0x0000ff00)/0x100,(int_color & 0x00ff0000)/0x10000,0};
//    #endif
//    return color;
//}

int drawText(SDL_Surface *surf, TTF_Font *font, SDL_Rect *rect, char *str, SDL_Color *color)
{
    SDL_Surface *img = TTF_RenderText_Blended(font,str, *color);
    SDL_Rect r1;
    SDL_GetClipRect(img, &r1);
    rect->w = r1.w;// * 0.8;
    rect->h = r1.h;// * 0.8;
    SDL_FillRect(spcsdl.mscr, rect, 0);
    SDL_BlitSurface(img, NULL, spcsdl.mscr, rect);
    SDL_FreeSurface(img);
    return rect->w;
}


void print_string(SDL_Surface *menu, char *cadena, int x, int y, unsigned char color, unsigned char back) {

	int length, ncarac, bucle, xx,yy, nr, width;
	int xxx, yyy;
	int w,h;
	unsigned char *str2;
	unsigned char str3[100];
    SDL_Surface *message;
    width = menu->w;
    if (spcsdl.font == NULL)
    {
        if (spconf.font_size == 0)
            spconf.font_size = 22;
        if (strlen(spconf.font_name) > 0)
            spcsdl.font = TTF_OpenFont(spconf.font_name, spconf.font_size);
        else
            spcsdl.font = TTF_OpenFont("JOYSTIX.TTF", spconf.font_size);
        TTF_SetFontHinting(spcsdl.font, TTF_HINTING_MONO);
        //printf("font created =%x\n", font);
    }

	if (spconf.text_mini==1) {
		if (x!=-1)
			x/=2;
		y/=2;
		w=8;
		h=10;
	} else {
		w=16;
		h=20;
		y=y*0.8-10;
		if (x >= 0)
            x=x*0.8;
	}

    int i = 0;
	for (ncarac=0,str2=cadena;*str2;str2++) {
		if (((*str2)>=' ')/*||(*str2==13)*/) {
            str3[ncarac++] = *str2;
            //printf("%c-%d\n", *str2, (int)*str2);
		}
	}
	str3[ncarac] = 0;
	TTF_SizeText(spcsdl.font,str3,&length,NULL);
    xx = (x == -1 ? ((width / 2) - (length / 2)) : x);
	//printf("%s(%s) w=%d, len=%d, xx=%d\n", str3, cadena, width, length, xx);
	nr=length/width;
	yy=y-h*nr;

	if (yy<0) yy=0;

	SDL_Rect rect;
    rect.y = yy;
	str2=cadena;
	if (spcsdl.font)
    {
        char str[100];
        int i = 0;
        for (bucle=0;bucle<ncarac;bucle++){
            while((*str2)<' ') {
                if (i > 0) {
                    str[i] = 0;
                    rect.x = xx;
                    xx += drawText(spcsdl.mscr, spcsdl.font, &rect, str, &spcsdl.colores[color]);
                    str[i=0] = 0;
                }
                if ((*str2)==1) {
                    color=*(str2+1);
                    printf("color=%d\n", color);
                    str2+=2;
                    continue;
                }
                if (*str2==2) {
                    back=*(str2+1);
                    str2+=2;
                    continue;
                }
                if (*str2==13) {
                    *(str2)=' ';
                    str2++;
                    continue;
                }
                str2++;
            }
            str[i++]=*str2++;
        }
        if (i > 0) {
            str[i] = 0;
            rect.x = xx;
            xx += drawText(spcsdl.mscr, spcsdl.font, &rect, str, &spcsdl.colores[color]);
        }
    }
    return;
}

int launch_menu(unsigned int key_pressed) {

	int retval=0;
	switch(key_pressed) {

//		case SDLK_F2:
//			snapshots_menu ();	// manage snapshot files
//			retval=1;
//			break;

		case SDLK_F3:
			taps_menu ();	// manage TAP files
			retval=1;
			break;

		case SDLK_F4:	// settings
			settings_menu ();
			retval=1;
			break;

		case SDLK_F7:
//			microdrive_menu ();	// shows the microdrive menu
			retval=1;
			break;

		case SDLK_F8:
//			tools_menu();
			retval=1;
			break;
	}
	return (retval);
}

void settings_menu() {

	unsigned char fin;
	unsigned char texto[41];

	fin=1;

	texto[0]=0;

	do {
		clean_screen();

		print_string(spcsdl.mscr,"Current settings",-1,20,15,0);
		sprintf(texto,"Mode: SPC-1000A");

		switch(spconf.mode) {
		case 0:
//			if(spconf.issue==2)
//				sprintf(texto,"Mode: SPC-1000A");
//			else
//				sprintf(texto,"Mode: SPC-10000 OLD");
		break;
		case 1:
			sprintf(texto,"Model: SPC-1000 NEW");
		break;
		case 2:
			sprintf(texto,"Model: SPC-1000A");
		break;
		case 3:
			sprintf(texto,"Model: SPC-1100");
		break;
		case 4:
			sprintf(texto,"Model: SPC-1000 Old");
		break;
		}

		print_string(spcsdl.mscr,texto,-1,45,14,0);

//		switch(spcsdl.joystick[0]) {
//		case 0:
//			sprintf(texto,"Joystick emulation: Cursor");
//			break;
//		case 1:
//			sprintf(texto,"Joystick emulation: Kempston");
//			break;
//		case 2:
//			sprintf(texto,"Joystick emulation: Sinclair (1)");
//			break;
//		case 3:
//			sprintf(texto,"Joystick emulation: Sinclair (2)");
//			break;
//		}
//		print_string(spcsdl.mscr,texto,-1,65,13,0);

		if(spconf.ay_emul)
			sprintf(texto,"AY-3-8912 Emulation: enabled");
		else
			sprintf(texto,"AY-3-8912 Emulation: disabled");

		print_string(spcsdl.mscr,texto,-1,85,11,0);

//		if(spcsdl.mdr_active)
//			sprintf(texto,"Interface I Emulation: enabled");
//		else
//			sprintf(texto,"Interface I Emulation: disabled");

//		print_string(spcsdl.mscr,texto,-1,105,15,0);

		if(spconf.dblscan)
			sprintf(texto,"Double scan: enabled");
		else
			sprintf(texto,"Double scan: disabled");

		print_string(spcsdl.mscr,texto,-1,125,12,0);

		if(spconf.turbo)
			sprintf(texto,"TURBO auto mode: enabled");
		else
			sprintf(texto,"TURBO auto mode: disabled");
		print_string(spcsdl.mscr,texto,-1,145,14,0);

		if (spconf.bw) {
			print_string(spcsdl.mscr,"TV Set: \001\011B\001\012&\001\014W",-1,165,15,0);
		} else {
			print_string(spcsdl.mscr,"TV Set: \001\012C\001\014o\001\015l\001\016o\001\013r",-1,165,15,0);
		}

		print_string(spcsdl.mscr,"1:",30,190,12,0);
		print_string(spcsdl.mscr,"SPC-1000 New",78,190,15,0);

		print_string(spcsdl.mscr,"2:",350,190,12,0);
		print_string(spcsdl.mscr,"SPC-1000A",398,190,15,0);

		print_string(spcsdl.mscr,"3:",30,220,12,0);
		print_string(spcsdl.mscr,"SPC-1100",78,220,15,0);

		print_string(spcsdl.mscr,"4:",350,220,12,0);
		print_string(spcsdl.mscr,"SPC-1000 Old",398,220,15,0);

		print_string(spcsdl.mscr,"5:",30,250,12,0);
		print_string(spcsdl.mscr,"Amstrad +2A/+3",78,250,15,0);

		print_string(spcsdl.mscr,"6:",350,250,12,0);
		print_string(spcsdl.mscr,"Spanish 128K",398,250,15,0);

		print_string(spcsdl.mscr,"7:",30,280,12,0);
		print_string(spcsdl.mscr,"Cursor",78,280,15,0);

		print_string(spcsdl.mscr,"8:",350,280,12,0);
		print_string(spcsdl.mscr,"Kempston",398,280,15,0);

		print_string(spcsdl.mscr,"9:",30,310,12,0);
		print_string(spcsdl.mscr,"Sinclair (1)",78,310,15,0);

		print_string(spcsdl.mscr,"0:",350,310,12,0);
		print_string(spcsdl.mscr,"Sinclair (2)",398,310,15,0);

		print_string(spcsdl.mscr,"I:",30,340,12,0);
		print_string(spcsdl.mscr,"Interface I",78,340,15,0);

		print_string(spcsdl.mscr,"D:",350,340,12,0);
		print_string(spcsdl.mscr,"Double Scan",398,340,15,0);

		print_string(spcsdl.mscr,"A:",350,370,12,0);
		print_string(spcsdl.mscr,"AY emulation",398,370,15,0);
		print_string(spcsdl.mscr,"T:",30,370,12,0);
		print_string(spcsdl.mscr,"TURBO mode",78,370,15,0);

		print_string(spcsdl.mscr,"V:",30,400,12,0);
		print_string(spcsdl.mscr,"TV Set mode",78,400,15,0);

		#ifndef GEKKO
		print_string(spcsdl.mscr,"F:",350,400,12,0);
		print_string(spcsdl.mscr,"Full Screen",398,400,15,0);
		#endif

		print_string(spcsdl.mscr,"ESC:",168,450,12,0);
		print_string(spcsdl.mscr,"return emulator",232,450,15,0);

        flip_screen(spcsdl.mscr);
		switch(wait_key()) {
		case SDLK_ESCAPE:
		case SDLK_RETURN:
			fin=0;
		break;
		case SDLK_1:
//			spcsdl.issue=2;
//			spcsdl.mode128k=0;
//			spcsdl.ay_emul=0;
//			ResetComputer();
		break;
		case SDLK_2:
//			spcsdl.issue=3;
//			spcsdl.mode128k=0;
//			spcsdl.ay_emul=0;
//			ResetComputer();
		break;
		case SDLK_3:
//			spcsdl.issue=3;
//			spcsdl.mode128k=1;
//			spcsdl.ay_emul=1;
//			spcsdl.videosystem=0;
//			ResetComputer();
		break;
		case SDLK_4:
//			spcsdl.issue=3;
//			spcsdl.mode128k=2;
//			spcsdl.ay_emul=1;
//			spcsdl.videosystem=0;
//			ResetComputer();
		break;
		case SDLK_5:
//			spcsdl.issue=3;
//			spcsdl.mode128k=3;
//			spcsdl.ay_emul=1;
//			spcsdl.videosystem=0;
//			spcsdl.mdr_active=0;
//			ResetComputer();
		break;
		case SDLK_6:
//			spcsdl.issue=3;
//			spcsdl.mode128k=4;
//			spcsdl.ay_emul=1;
//			spcsdl.videosystem=0;
//			ResetComputer();
		break;
		case SDLK_7:
//			spcsdl.joystick[0]=0;
		break;
		case SDLK_8:
//			spcsdl.joystick[0]=1;
		break;
		case SDLK_9:
//			spcsdl.joystick[0]=2;
		break;
		case SDLK_0:
//			spcsdl.joystick[0]=3;
		break;
		case SDLK_i:
//			if(spcsdl.mode128k!=3) {
//				spcsdl.mdr_active=1-spcsdl.mdr_active;
//				ResetComputer();
//			}
		break;
		case SDLK_d:
//			spcsdl.dblscan=1-spcsdl.dblscan;
//			update_npixels();
		break;
		case SDLK_a:
			spconf.ay_emul=1-spconf.ay_emul;
		break;
		case SDLK_v:
//			spcsdl.bw=1-spcsdl.bw;
//			computer_set_palete();
		break;
		case SDLK_t:
//			curr_frames=0;
//			update_frequency(0); //set deafult machine frequency
//			jump_frames=0;
//			spcsdl.turbo_state = 0;
//			if(spcsdl.turbo){
//				spcsdl.turbo = 0;
//			} else {
//				spcsdl.turbo = 1; //Auto mode
//			}
		break;
		#ifndef GEKKO
		case SDLK_f:
//			SDL_Fullspcsdl.mscr_Switch();
		break;
		#endif
		}
	} while(fin);

	clean_screen();
}

void init_menu()
{
    if (spcsdl.mscr == NULL)
    {
        SDL_Surface *emul = spcsdl.emul;
        spcsdl.mscr =  SDL_CreateRGBSurface(SDL_SWSURFACE, 256*2, 192*2, emul->format->BitsPerPixel, emul->format->Rmask, emul->format->Gmask, emul->format->Bmask, 0);
        printf("BitsPerPixel=%d\n", emul->format->BitsPerPixel);
    }
//    colors[0]=SDL_MapRGB(spcsdl.mscr->format,0,0,0);
//    colors[1]=SDL_MapRGB(spcsdl.mscr->format,0,0,192);
//    colors[2]=SDL_MapRGB(spcsdl.mscr->format,192,0,0);
//    colors[3]=SDL_MapRGB(spcsdl.mscr->format,192,0,192);
//    colors[4]=SDL_MapRGB(spcsdl.mscr->format,0,192,0);
//    colors[5]=SDL_MapRGB(spcsdl.mscr->format,0,192,192);
//    colors[6]=SDL_MapRGB(spcsdl.mscr->format,192,192,0);
//    colors[7]=SDL_MapRGB(spcsdl.mscr->format,192,192,192);
//    colors[8]=SDL_MapRGB(spcsdl.mscr->format,0,0,0);
//    colors[9]=SDL_MapRGB(spcsdl.mscr->format,0,0,255);
//    colors[10]=SDL_MapRGB(spcsdl.mscr->format,255,0,0);
//    colors[11]=SDL_MapRGB(spcsdl.mscr->format,255,0,255);
//    colors[12]=SDL_MapRGB(spcsdl.mscr->format,0,255,0);
//    colors[13]=SDL_MapRGB(spcsdl.mscr->format,0,255,255);
//    colors[14]=SDL_MapRGB(spcsdl.mscr->format,255,255,0);
//    colors[15]=SDL_MapRGB(spcsdl.mscr->format,255,255,255);

    if (spconf.bw == 0)
    {
		spcsdl.colores[0].r = 0;
		spcsdl.colores[0].g = 0;
		spcsdl.colores[0].b = 0;
		spcsdl.colores[1].r = 0;
		spcsdl.colores[1].g = 0;
		spcsdl.colores[1].b = 192;
		spcsdl.colores[2].r = 192;
		spcsdl.colores[2].g = 0;
		spcsdl.colores[2].b = 0;
		spcsdl.colores[3].r = 192;
		spcsdl.colores[3].g = 0;
		spcsdl.colores[3].b = 192;
		spcsdl.colores[4].r = 0;
		spcsdl.colores[4].g = 192;
		spcsdl.colores[4].b = 0;
		spcsdl.colores[5].r = 0;
		spcsdl.colores[5].g = 192;
		spcsdl.colores[5].b = 192;
		spcsdl.colores[6].r = 192;
		spcsdl.colores[6].g = 192;
		spcsdl.colores[6].b = 0;
		spcsdl.colores[7].r = 192;
		spcsdl.colores[7].g = 192;
		spcsdl.colores[7].b = 192;
		spcsdl.colores[8].r = 0;
		spcsdl.colores[8].g = 0;
		spcsdl.colores[8].b = 0;
		spcsdl.colores[9].r = 0;
		spcsdl.colores[9].g = 0;
		spcsdl.colores[9].b = 255;
		spcsdl.colores[10].r = 255;
		spcsdl.colores[10].g = 0;
		spcsdl.colores[10].b = 0;
		spcsdl.colores[11].r = 255;
		spcsdl.colores[11].g = 0;
		spcsdl.colores[11].b = 255;
		spcsdl.colores[12].r = 0;
		spcsdl.colores[12].g = 255;
		spcsdl.colores[12].b = 0;
		spcsdl.colores[13].r = 0;
		spcsdl.colores[13].g = 255;
		spcsdl.colores[13].b = 255;
		spcsdl.colores[14].r = 255;
		spcsdl.colores[14].g = 255;
		spcsdl.colores[14].b = 0;
		spcsdl.colores[15].r = 255;
		spcsdl.colores[15].g = 255;
		spcsdl.colores[15].b = 255;
    }
}
// shows the help menu

void help_menu() {

	unsigned char fin;

	clean_screen();

	print_string(spcsdl.mscr,"fbSPC (1.0)",-1,20,15,0);
	print_string(spcsdl.mscr,"Available keys",-1,50,14,0);
	print_string(spcsdl.mscr,"Alt:Graph    F1~F5:function key",-1,95,11,0);

	print_string(spcsdl.mscr,"F6:",14,160,12,0);
	print_string(spcsdl.mscr,"this help",62,160,15,0);

	print_string(spcsdl.mscr,"@F6:",326,160,12,0);
	print_string(spcsdl.mscr,"manage snapshots",402,160,15,0);

	print_string(spcsdl.mscr,"F7:",14,200,12,0);
	print_string(spcsdl.mscr,"manage TAP/SPC",62,200,15,0);

	print_string(spcsdl.mscr,"@F7:",326,200,12,0);
	print_string(spcsdl.mscr,"change settings",402,200,15,0);

	print_string(spcsdl.mscr,"F8:",14,240,12,0);
	print_string(spcsdl.mscr,"play TAPE",62,240,15,0);

	print_string(spcsdl.mscr,"@F8:",326,240,12,0);
	print_string(spcsdl.mscr,"stop TAPE",402,240,15,0);

	print_string(spcsdl.mscr,"F9:",14,280,12,0);
	print_string(spcsdl.mscr,"manage FDD",62,280,15,0);

	print_string(spcsdl.mscr,"@F9:",326,280,12,0);
	print_string(spcsdl.mscr,"tools",402,280,15,0);

	print_string(spcsdl.mscr,"F10:",14,320,12,0);
	print_string(spcsdl.mscr,"\001\016quit",72,320,15,0);

	print_string(spcsdl.mscr,"@F10:",326,320,12,0);
	print_string(spcsdl.mscr,"reset SPC-1000",408,320,15,0);

	print_string(spcsdl.mscr,"F11/O:",14,360,12,0);
	print_string(spcsdl.mscr,"volume low",110,360,15,0);

	print_string(spcsdl.mscr,"F12/P:",326,360,12,0);
	print_string(spcsdl.mscr,"volume up",408,360,15,0);

	print_string(spcsdl.mscr,"ESC: return to emulator",-1,400,12,0);
	print_copy(spcsdl.mscr);

    flip_screen(spcsdl.mscr);
	fin=1;
	do {
		unsigned int key = wait_key();
		switch(key) {
		case SDLK_ESCAPE: // to exit the help
			fin=0;
		break;
		default:
			if (launch_menu(key)) {
				fin=0;
			}
		break;
		}
	} while(fin);
	//clean_screen();
	//printf("clean_screen()\n");
}


// shows the POKE menu

//void do_poke() {
//
//	unsigned char *videomem,string[80];
//	int ancho,retorno,address,old_value,new_value;
//
//	videomem=spcsdl.mscr->pixels;
//	ancho=spcsdl.mscr->w;
//
//	clean_screen();
//
//	while(1) {
//		print_string(videomem,"Type address to POKE",-1,32,15,0);
//		print_string(videomem,"(ESC to exit)",-1,52,12,0);
//
//		retorno=ask_value(&address,84,65535);
//
//		clean_screen();
//
//		if (retorno==2) {
//			return;
//		}
//
//		if ((address<16384) && ((spcsdl.mode128k != 3) || (1 != (spcsdl.mport2 & 0x01)))) {
//			print_string(videomem,"That address is ROM memory.",-1,13,15,0);
//			continue;
//		}
//
//		switch (address & 0x0C000) {
//		case 0x0000:
//			old_value= (*(spcsdl.block0 + address));
//		break;
//
//		case 0x4000:
//			old_value= (*(spcsdl.block1 + address));
//		break;
//
//		case 0x8000:
//			old_value= (*(spcsdl.block2 + address));
//		break;
//
//		case 0xC000:
//			old_value= (*(spcsdl.block3 + address));
//		break;
//		default:
//			old_value=0;
//		break;
//		}
//
//		print_string(videomem,"Type new value to POKE",-1,32,15,0);
//		print_string(videomem,"(ESC to cancel)",-1,52,12,0);
//		sprintf(string,"Address: %d; old value: %d\n",address,old_value);
//		print_string(videomem,string,-1,130,14,0);
//
//		retorno=ask_value(&new_value,84,255);
//
//		clean_screen();
//
//		if (retorno==2) {
//			continue;
//		}
//
//		switch (address & 0x0C000) {
//		case 0x0000:
//			(*(spcsdl.block0 + address))=new_value;
//		break;
//
//		case 0x4000:
//			(*(spcsdl.block1 + address))=new_value;
//		break;
//
//		case 0x8000:
//			(*(spcsdl.block2 + address))=new_value;
//		break;
//
//		case 0xC000:
//			(*(spcsdl.block3 + address))=new_value;
//		break;
//		default:
//		break;
//		}
//
//		sprintf(string,"Set address %d from %d to %d\n",address,old_value,new_value);
//		print_string(videomem,string,-1,130,14,0);
//
//	}
//}

// shows the tools menu

//void tools_menu() {
//
//	unsigned char *spcsdl.mscr,fin;
//	int ancho=spcsdl.mscr->w;
//
//	spcsdl.mscr=spcsdl.mscr->pixels;
//
//	fin=1;
//	do {
//		clean_screen();
//
//		print_string(spcsdl.mscr,"Tools",-1,20,15,0);
//
//		print_string(spcsdl.mscr,"1:",14,60,12,0);
//		print_string(spcsdl.mscr,"show keyboard template",62,60,15,0);
//
//		print_string(spcsdl.mscr,"2:",14,100,12,0);
//		print_string(spcsdl.mscr,"insert POKEs",62,100,15,0);
//
//		print_string(spcsdl.mscr,"ESC:",14,250,12,0);
//		print_string(spcsdl.mscr,"return emulator",78,250,15,0);
//
//		//print_copy(spcsdl.mscr);
//
//		switch(wait_key()) {
//		case SDLK_ESCAPE: // to exit the help
//			fin=0;
//		break;
//		case SDLK_1:
//			fin=0;
//			keyboard_menu();
//		break;
//		case SDLK_2:
//			fin=0;
//			do_poke();
//		break;
//		default:
//		break;
//		}
//
//	} while(fin);
//
//	clean_screen();
//}




// shows the SNAPSHOTS menu

void snapshots_menu() {

	unsigned char fin;

	clean_screen();

	print_string(spcsdl.mscr,"SNAPSHOTS",-1,30,15,0);

	print_string(spcsdl.mscr,"1:",14,100,12,0);
	print_string(spcsdl.mscr,"load a S1S snapshot",62,100,15,0);

	print_string(spcsdl.mscr,"2:",14,160,12,0);
	print_string(spcsdl.mscr,"make a S1S snapshot",62,160,15,0);

	print_string(spcsdl.mscr,"3: \001\013load a SCR snapshot",14,220,12,0);

	print_string(spcsdl.mscr,"4: \001\013save a SCR snapshot",14,280,12,0);

	print_string(spcsdl.mscr,"ESC: \001\013return to emulator",-1,400,12,0);

	print_copy(spcsdl.mscr);

    flip_screen(spcsdl.mscr);

	fin=1;
	do {
		switch(wait_key()) {
		case SDLK_ESCAPE: // to exit the help
			fin=0;
		break;
		case SDLK_1:
			fin=0;
//			load_z80file();
		break;
		case SDLK_2:
			fin=0;
//			save_z80file();
		break;
		case SDLK_3:
			fin=0;
//			load_scrfile();
		break;
		case SDLK_4:
			fin=0;
//			create_scrfile();
		break;
		default:
		break;
		}
	} while(fin);
	clean_screen();
}


// shows the TAPs menu

void taps_menu() {

	unsigned char fin;

	fin=1;
	do {
		clean_screen();

		print_string(spcsdl.mscr,"TAP/SPC files",-1,20,15,0);

		print_string(spcsdl.mscr,"1:",14,60,12,0);
		print_string(spcsdl.mscr,"select a TAP/SPC file",62,60,15,0);


		print_string(spcsdl.mscr,"2:",14,100,12,0);
		print_string(spcsdl.mscr,"rewind TAP/SPC file",62,100,15,0);

		print_string(spcsdl.mscr,"3:",14,140,12,0);
		print_string(spcsdl.mscr,"instant/normal load",62,140,15,0);

		print_string(spcsdl.mscr,"4:",14,180,12,0);
		print_string(spcsdl.mscr,"write protection",62,180,15,0);

		print_string(spcsdl.mscr,"5:",14,220,12,0);
		print_string(spcsdl.mscr,"create TAP file",62,220,15,0);

		print_string(spcsdl.mscr,"ESC:",14,260,12,0);
		print_string(spcsdl.mscr,"return emulator",78,260,15,0);

		print_string(spcsdl.mscr,"Current TAP/SPC file is:",-1,310,12,0);
		print_string(spcsdl.mscr,spconf.current_tap,-1,330,12,0);

		print_copy(spcsdl.mscr);

		if(spconf.casTurbo)
			print_string(spcsdl.mscr,"Fast load enabled	",10,420,14,0);
		else
			print_string(spcsdl.mscr,"Fast load disabled ",10,420,14,0);

		if(spconf.tape_write)
			print_string(spcsdl.mscr,"Write enabled",390,420,14,0);
		else
			print_string(spcsdl.mscr,"Write disabled",390,420,14,0);

        flip_screen(spcsdl.mscr);

		switch(wait_key()) {
		case SDLK_ESCAPE: // to exit the help
			fin=0;
		break;
		case SDLK_1: //select tape
			spconf.tape_stop=1;
			spconf.tape_stop_fast = 1;
			//spconf.tape_start_countdwn=0;
			select_tapfile();
		break;
		case SDLK_2: //rewind tape
			fin=0;
			spconf.tape_stop=1;
			spconf.tape_stop_fast = 1;
			//spconf.tape_start_countdwn=0;
			if(spconf.rfp!=NULL) {
				//spconf.tape_current_mode=TAP_TRASH;
//				rewind_tape(spconf.tap_file,1);
			}
//			sprintf(spconf.osd_text,"Tape rewound");
			//spconf.osd_time=50;
		break;
		case SDLK_3: //Instant load settings
			spconf.tape_stop=1;
			spconf.tape_stop_fast = 1;
//			spconf.tape_start_countdwn=0;
			spconf.casTurbo=1-spconf.casTurbo;
			if(spconf.tap_file!=NULL) {
//				spconf.tape_current_mode=TAP_TRASH;
//				rewind_tape(spconf.tap_file,1);
			}
		break;
		case SDLK_4:
			spconf.tape_write=1-spconf.tape_write;
		break;
		case SDLK_5:
			create_tapfile();
		break;
		default:
		break;
		}

	} while(fin);

	clean_screen();
}

// shows a menu to allow user to choose a tape file

void select_tapfile() {

	unsigned char *filename;
	int ancho,retorno,retval;
	unsigned char char_id[11];

	//videomem=spcsdl.mscr->pixels;
	//ancho=spcsdl.mscr->w;

	clean_screen();

	print_string(spcsdl.mscr,"Choose the TAPE file to load",-1,32,13,0);

	filename=select_file("../cas",FILETYPE_TAP_SPC);

	if(filename==NULL) { // Aborted
		clean_screen();
		return;
	}

//	spcsdl.tape_current_bit=0;
//	spcsdl.tape_current_mode=TAP_TRASH;
//	spcsdl.next_block= NOBLOCK;

	if(spconf.tap_file!=NULL) {
		fclose(spconf.tap_file);
	}

	if (!strncmp(filename,"smb:",4)) spconf.tap_file=fopen(filename,"r"); //tinysmb does not work with r+
	else spconf.tap_file=fopen(filename,"r+b"); // read and write
	spconf.tape_write = 0; // by default, can't record

	if(spconf.tap_file==NULL)
		retorno=-1;
	else
		retorno=0;

	clean_screen();
    printf("current_tap-%s\n", filename);
	strcpy(spconf.current_tap,filename);

	free(filename);

	switch(retorno) {
	case 0: // all right
	break;
	case -1:
		print_string(spcsdl.mscr,"Error: Can't load that file",-1,232,10,0);
		print_string(spcsdl.mscr,"Press any key",-1,248,10,0);
		spconf.current_tap[0]=0;
		wait_key();
	break;
	}

	retval=fread(char_id,10,1,spconf.tap_file); // read the (maybe) SPC header
	if((!strncmp(char_id,"ZXTape!",7)) && (char_id[7]==0x1A)&&(char_id[8]==1)) {
		spconf.tape_file_type = TAP_SPC;
		//create_browser_spc(spconf.tap_file);
	} else {
		spconf.tape_file_type = TAP_TAP;
		//spconf(spconf.tap_file);
	}

	clean_screen();
}

void create_tapfile() {

	//unsigned char *videomem;
	int ancho,retorno;
	unsigned char nombre2[MAX_PATH_LENGTH];

	//videomem=spcsdl.mscr->pixels;
	//ancho=spcsdl.mscr->w;

	clean_screen();

	print_string(spcsdl.mscr,"Choose a name for the TAP file",-1,32,14,0);
	print_string(spcsdl.mscr,"(up to 30 characters)",-1,52,14,0);

	print_string(spcsdl.mscr,"TAP file will be saved in:",-1,132,12,0);
	print_string(spcsdl.mscr,path_taps,0,152,12,0);


	retorno=ask_filename(nombre2,84,"tap",path_taps,NULL);

	clean_screen();

	if(retorno==2) // abort
		return;

	if(spconf.wfp!=NULL)
		fclose(spconf.wfp);

	spconf.wfp=fopen(nombre2,"r"); // test if it exists
	if(spconf.wfp==NULL)
		retorno=0;
	else
		retorno=-1;

	if(!retorno) {
		spconf.wfp=fopen(nombre2,"a+b"); // create for read and write
		if(spconf.wfp==NULL)
			retorno=-2;
		else
			retorno=0;
	}
	spconf.tape_write=1; // allow to write
	printf("current_tap=%s\n", nombre2);
	strcpy(spconf.current_tap,nombre2);
	spconf.tape_file_type = TAP_TAP;
	switch(retorno) {
	case 0:
	strcpy(spconf.last_selected_file,nombre2);
//	create_browser_tap(spconf.wfp);
	break;
	case -1:
		print_string(spcsdl.mscr,"File already exists",-1,80,10,0);
//		spcsdl.current_tap[0]=0;
		wait_key();
	break;
	case -2:
		print_string(spcsdl.mscr,"Can't create file",-1,80,10,0);
		spconf.current_tap[0]=0;
		wait_key();
	break;
	}
	clean_screen();
}

// shows the microdrive menu

void floppy_disk_menu() {

	unsigned char fin;
	int retval,ancho=spcsdl.mscr->w;
	FILE *fd;

	fin=1;
	do {
			clean_screen();

		print_string(spcsdl.mscr,"Floppy Disk files",-1,20,15,0);

		print_string(spcsdl.mscr,"1:",14,60,12,0);
		print_string(spcsdl.mscr,"select a disk file",62,60,15,0);

		print_string(spcsdl.mscr,"2:",14,100,12,0);
		print_string(spcsdl.mscr,"create a disk file",62,100,15,0);

		print_string(spcsdl.mscr,"3:",14,140,12,0);
		print_string(spcsdl.mscr,"save the disk file",62,140,15,0);

		print_string(spcsdl.mscr,"ESC:",14,180,12,0);
		print_string(spcsdl.mscr,"return emulator",78,180,15,0);

		print_string(spcsdl.mscr,"Current disk file is:",-1,300,12,0);
		print_string(spcsdl.mscr,spcsys.fdd.diskfile,-1,320,12,0);

		//print_copy(spcsdl.mscr);

		if(!spcsys.fdd.write)
			print_string(spcsdl.mscr,"Write enabled",-1,420,14,0);
		else
			print_string(spcsdl.mscr,"Write disabled",-1,420,14,0);

		switch(wait_key()) {
		case SDLK_ESCAPE: // to exit the help
			fin=0;
		break;

		case SDLK_1:
			select_diskfile();
		break;
		case SDLK_2:
			create_diskfile();
		break;
		case SDLK_3:
			if(spcsys.fdd.write)
				spcsys.fdd.write=0;
			else
				spcsys.fdd.write=1;
			fd=fopen(spcsys.fdd.diskfile,"wb"); // create for write
			if(fd!=NULL) {
				retval=fwrite(spcsys.fdd.diskdata,1,sizeof(spcsys.fdd.diskdata),fd); // save cartridge
				fclose(fd);
				fd=NULL;
				spcsys.fdd.modified=0;
			}
		break;
		default:
		break;
		}

	} while(fin);

	clean_screen();
}
//
// shows a menu to allow user to choose a microdrive file

void select_diskfile() {

	unsigned char *filename;
	int ancho,retorno,retval;
	FILE *fd;
	// unsigned char char_id[11];


    clean_screen();

	print_string(spcsdl.mscr,"Choose the DISK file to load",-1,32,13,0);

	filename=select_file(path_disks,FILETYPE_DISK); // MDR files

	if(filename==NULL) { // Aborted
		clean_screen();
		return;
	}

	fd=fopen(filename,"rb"); // read
	if(fd==NULL)
		retorno=-1;
	else {
		retorno=0;
		retval=fread(spcsys.fdd.diskdata,sizeof(spcsys.fdd.diskdata),1,fd); // read the cartridge in memory
		spcsys.fdd.modified=0; // not modified
		fclose(fd);
	}

	clean_screen();

	strcpy(spcsys.fdd.diskfile,filename);

	free(filename);

	switch(retorno) {
	case 0: // all right
		break;
	case -1:
		print_string(spcsdl.mscr,"Error: Can't load that file",-1,232,10,0);
		print_string(spcsdl.mscr,"Press any key",-1,248,10,0);
		spcsys.fdd.diskfile[0]=0;
		wait_key();
		break;
	}

	clean_screen();
}

void create_diskfile() {

	unsigned char *videomem;
	int ancho,retorno,bucle,retval;
	unsigned char nombre2[MAX_PATH_LENGTH];
	FILE *fd;

	videomem=spcsdl.mscr->pixels;
	ancho=spcsdl.mscr->w;

	clean_screen();

	print_string(spcsdl.mscr,"Choose a name for the disk file",-1,32,14,0);
	print_string(spcsdl.mscr,"(up to 30 characters)",-1,52,14,0);

	print_string(spcsdl.mscr,"disk file will be saved in:",-1,132,12,0);
	print_string(spcsdl.mscr,path_disks,0,152,12,0);

	retorno=ask_filename(nombre2,84,"dsk",path_disks, NULL);

	clean_screen();

	if(retorno==2) // abort
		return;

	fd=fopen(nombre2,"r"); // test if it exists
	if(fd==NULL)
		retorno=0;
	else
		retorno=-1;

	if(!retorno) {
		fd=fopen(nombre2,"wb"); // create for write
		if(fd==NULL)
			retorno=-2;
		else {
//			for(bucle=0;bucle<137921;bucle++)
//				spcsdl.mdr_cartridge[bucle]=0xFF; // erase cartridge
//			spcsdl.mdr_cartridge[137922]=0;
//			retval=fwrite(spcsdl.mdr_cartridge,137923,1,spcsdl.mdr_file); // save cartridge
//			fclose(spcsdl.mdr_file);
//			spcsdl.mdr_file=NULL;
//			spcsdl.mdr_modified=0;
//			retorno=0;
		}
	}
	strcpy(spcsys.fdd.diskfile,nombre2);
	switch(retorno) {
	case 0:
	break;
	case -1:
		print_string(spcsdl.mscr,"File already exists",-1,80,10,0);
		spcsys.fdd.diskfile[0]=0;
		wait_key();
	break;
	case -2:
		print_string(spcsdl.mscr,"Can't create file",-1,80,10,0);
		spcsys.fdd.diskfile[0]=0;
		wait_key();
	break;
	}
	clean_screen();
}


//void create_scrfile() {
//
//	unsigned char *videomem;
//	int ancho,retorno,retval;
//	unsigned char nombre2[MAX_PATH_LENGTH];
//	FILE *fichero;
//	char *name;
//
//	videomem=spcsdl.mscr->pixels;
//	ancho=spcsdl.mscr->w;
//
//	clean_screen();
//
//	print_string(videomem,"Choose a name for the SCR file",-1,32,14,0);
//	print_string(videomem,"(up to 30 characters)",-1,52,14,0);
//
//	print_string(videomem,"SCR file will be saved in:",-1,132,12,0);
//	print_string(videomem,path_scr1,0,152,12,0);
//
//	if (strlen(spcsdl.current_tap))
//		{
//		name=strrchr(spcsdl.current_tap,'/');
//		if (name) name++; else name = spcsdl.current_tap;
//		}
//	else
//	 name=NULL;
//
//	retorno=ask_filename(nombre2,84,"scr",path_scr1, name);
//
//	clean_screen();
//
//	if(retorno==2) // abort
//		return;
//
//	fichero=fopen(nombre2,"r"); // test if it exists
//	if(fichero==NULL)
//		retorno=0;
//	else {
//		fclose(fichero);
//		retorno=-1;
//	}
//
//	if(!retorno) {
//		fichero=fopen(nombre2,"wb"); // create for write
//		if(fichero==NULL)
//			retorno=-2;
//		else {
//			retval=fwrite(spcsdl.block1+0x04000,6912,1,fichero); // save spcsdl.mscr
//			if (spcsdl.ulaplus!=0) {
//				retval=fwrite(spcsdl.ulaplus_palete,64,1,fichero); // save ULAPlus palete
//			}
//			fclose(fichero);
//			retorno=0;
//		}
//	}
//
//	switch(retorno) {
//	case -1:
//		print_string(videomem,"File already exists",-1,80,10,0);
//		wait_key();
//	break;
//	case -2:
//		print_string(videomem,"Can't create file",-1,80,10,0);
//		wait_key();
//	break;
//	default:
//	break;
//	}
//	clean_screen();
//}


int ask_filename(char *nombre_final,int y_coord,char *extension, char *path, char *name) {

	int longitud,retorno;
	unsigned char nombre[37],nombre2[38];
	char *ptr;
	const char cursor = '_';

	//unsigned char *videomem;
	//int ancho;

	//videomem=spcsdl.mscr->pixels;
	//ancho=spcsdl.mscr->w;

	retorno=0;

	if (!name||(strlen(name)>36))
    {
		nombre[0]=cursor;
		nombre[1]=0;
    }
	else
    {
		strcpy(nombre,name);
		ptr = strrchr ((const char*)nombre, '.');
		if (ptr) //remove the extension
			{
			*ptr = cursor;
			*(ptr+1) = 0;
			}
		else
		nombre[strlen(nombre)-1]=cursor;
		nombre[strlen(nombre)]=0;
    }

	longitud=strlen(nombre)-1;


	do {
		sprintf (nombre2, " %s.%s ", nombre, extension);
		print_string (spcsdl.mscr, nombre2, -1, y_coord, 15, 0);
		int key = wait_key ();
		switch (key) {
		case SDLK_BACKSPACE:
			if (longitud > 0) {
				nombre[longitud]=0;
				longitud--;
				nombre[longitud]=cursor;
			}
		break;
		case SDLK_ESCAPE:
			retorno=2;
		break;
		case SDLK_RETURN:
			retorno=1;
		break;
		case SDLK_a:
		case SDLK_b:
		case SDLK_c:
		case SDLK_d:
		case SDLK_e:
		case SDLK_f:
		case SDLK_g:
		case SDLK_h:
		case SDLK_i:
		case SDLK_j:
		case SDLK_k:
		case SDLK_l:
		case SDLK_m:
		case SDLK_n:
		case SDLK_o:
		case SDLK_p:
		case SDLK_q:
		case SDLK_r:
		case SDLK_s:
		case SDLK_t:
		case SDLK_u:
		case SDLK_v:
		case SDLK_w:
		case SDLK_x:
		case SDLK_y:
		case SDLK_z:
			if (longitud < 30) {
				nombre[longitud++]='a' + (key - SDLK_a);
				nombre[longitud]=cursor;
				nombre[longitud + 1]=0;
			}
		break;
		case SDLK_0:
		case SDLK_1:
		case SDLK_2:
		case SDLK_3:
		case SDLK_4:
		case SDLK_5:
		case SDLK_6:
		case SDLK_7:
		case SDLK_8:
		case SDLK_9:
			if (longitud < 30) {
				nombre[longitud++]='0' + (key - SDLK_0);
				nombre[longitud]=cursor;
				nombre[longitud + 1]=0;
			}
		break;
		case SDLK_MINUS:
			if (longitud < 30) {
				nombre[longitud++]='-';
				nombre[longitud]=cursor;
				nombre[longitud + 1]=0;
			}
		break;
		case SDLK_UNDERSCORE:
			if (longitud < 30) {
				nombre[longitud++]='_';
				nombre[longitud]=cursor;
				nombre[longitud + 1]=0;
			}
		break;
		}
	} while (!retorno);

	nombre[longitud]=0; // erase cursor

	longitud=strlen(path);
	if((path[longitud-1]!='/')&&(longitud>=1))
		sprintf(nombre_final,"%s/%s.%s",path,nombre,extension); // name
	else
		sprintf(nombre_final,"%s%s.%s",path,nombre,extension);
	return (retorno);
}




//int ask_value(int *final_value,int y_coord,int max_value) {
//
//	unsigned char nombre2[50];
//	unsigned char *videomem;
//	int ancho,value,tmp,retorno;
//
//	videomem=spcsdl.mscr->pixels;
//	ancho=spcsdl.mscr->w;
//
//	retorno=0;
//	value=0;
//	do {
//		sprintf (nombre2, " %d\177 ", value);
//		print_string (videomem, nombre2, -1, y_coord, 15, 0, ancho);
//		switch (wait_key ()) {
//		case SDLK_BACKSPACE:
//			value/=10;
//		break;
//		case SDLK_ESCAPE:
//			retorno=2;
//		break;
//		case SDLK_RETURN:
//			retorno=1;
//		break;
//		case SDLK_0:
//			tmp=value * 10;
//			if (tmp <= max_value) {
//				value=tmp;
//			}
//		break;
//		case SDLK_1:
//			tmp=1+value * 10;
//			if (tmp <= max_value) {
//				value=tmp;
//			}
//		break;
//		case SDLK_2:
//			tmp=2+value * 10;
//			if (tmp <= max_value) {
//				value=tmp;
//			}
//		break;
//		case SDLK_3:
//			tmp=3+value * 10;
//			if (tmp <= max_value) {
//				value=tmp;
//			}
//		break;
//		case SDLK_4:
//			tmp=4+value * 10;
//			if (tmp <= max_value) {
//				value=tmp;
//			}
//		break;
//		case SDLK_5:
//			tmp=5+value * 10;
//			if (tmp <= max_value) {
//				value=tmp;
//			}
//		break;
//		case SDLK_6:
//			tmp=6+value * 10;
//			if (tmp <= max_value) {
//				value=tmp;
//			}
//		break;
//		case SDLK_7:
//			tmp=7+value * 10;
//			if (tmp <= max_value) {
//				value=tmp;
//			}
//		break;
//		case SDLK_8:
//			tmp=8+value * 10;
//			if (tmp <= max_value) {
//				value=tmp;
//			}
//		break;
//		case SDLK_9:
//			tmp=9+value * 10;
//			if (tmp <= max_value) {
//				value=tmp;
//			}
//		break;
//		}
//	} while (!retorno);
//
//	*final_value=value;
//
//	return (retorno);
//}




// shows a menu to allow user to save a snapshot file

void save_s1sfile() {

//	unsigned char *videomem;
	int ancho,retorno;
	unsigned char nombre2[MAX_PATH_LENGTH];
	char *name;

//	videomem=spcsdl.mscr->pixels;
//	ancho=spcsdl.mscr->w;

	clean_screen();

	print_string(spcsdl.mscr,"Choose a name for the S1S snapshot",-1,32,14,0);
	print_string(spcsdl.mscr,"(up to 30 characters)",-1,52,14,0);

	print_string(spcsdl.mscr,"Snapshot will be saved in:",-1,132,12,0);
	print_string(spcsdl.mscr,spconf.path_snaps,0,152,12,0);

	if (strlen(spconf.path_snaps))
    {
		name=strrchr(spconf.path_snaps,'/');
		if (name) name++; else name = spconf.path_snaps;
	}
	else
        name=NULL;

	retorno=ask_filename(nombre2,84,"S1S", spconf.path_snaps, name);

	clean_screen();

	if(retorno==2) // abort
		return;

    printf("save to = %s\n", nombre2);
	retorno=SaveImageFile(nombre2);
	switch(retorno) {
	case 0:
		break;
	case -1:
		print_string(spcsdl.mscr,"File already exists",-1,80,10,0);
		wait_key();
		break;
	case -2:
		print_string(spcsdl.mscr,"Can't create file",-1,80,10,0);
		wait_key();
		break;
	}
	clean_screen();
}
//
// shows a menu to allow user to load a snapshot file

void load_s1sfile() {


	unsigned char *filename;
	int ancho,retorno;

	clean_screen();

	print_string(spcsdl.mscr,"Choose the Z80 snapshot file to load",-1,32,13,0);

	filename=select_file(spconf.path_snaps, FILETYPE_S1S);

	if(filename==NULL) { // Aborted
		clean_screen();
		return;
	}

	retorno=LoadImageFile(filename);
	printf("load_s1s_file = %s(%d)\n", filename, retorno);
	free(filename);
	clean_screen();

	switch(retorno) {
	case 0: // all right
		break;
	case -1:
		print_string(spcsdl.mscr,"Error: Can't load that file",-1,232,10,0);
		print_string(spcsdl.mscr,"Press any key",-1,248,10,0);
		wait_key();
		break;
	case -2:
	case -3:
		print_string(spcsdl.mscr,"Error: unsupported snap file",-1,232,10,0);
		print_string(spcsdl.mscr,"Press any key",-1,248,10,0);
		wait_key();
		break;
	}
	clean_screen();
}

//
//void load_scrfile() {
//
//
//	unsigned char *videomem,*filename,value;
//	int ancho,retorno,loop;
//	FILE *fichero;
//	unsigned char paleta_tmp[64];
//
//	videomem=spcsdl.mscr->pixels;
//	ancho=spcsdl.mscr->w;
//
//	clean_screen();
//
//	print_string(videomem,"Choose the SCR snapshot file to load",-1,32,13,0);
//
//	filename=select_file(load_path_scr1,FILETYPE_SCR);
//
//	if(filename==NULL) { // Aborted
//		clean_screen();
//		return;
//	}
//
//	spcsdl.osd_text[0]=0;
//	fichero=fopen(filename,"rb");
//	retorno=0;
//	if (!fichero) {
//		retorno=-1;
//	} else {
//		for(loop=0;loop<6912;loop++) {
//			if (1==fread(&value,1,1,fichero)) {
//				*(spcsdl.block1 + 0x04000 + loop) = value;
//			} else {
//				retorno=-1;
//				break;
//			}
//		}
//		if (1==fread(paleta_tmp,64,1,fichero)) {
//			memcpy(spcsdl.ulaplus_palete,paleta_tmp,64);
//			spcsdl.ulaplus=1;
//		} else {
//			spcsdl.ulaplus=0;
//		}
//		fclose(fichero);
//	}
//
//	free(filename);
//	clean_screen();
//
//	switch(retorno) {
//	case 0: // all right
//		break;
//	default:
//		print_string(videomem,"Error: Can't load that file",-1,232,10,0);
//		print_string(videomem,"Press any key",-1,248,10,0);
//		wait_key();
//	break;
//	}
//	clean_screen();
//}



/* fills a FICHERO chained list with all the files and directories contained in PATH.
	 If KIND is 0, it returns only Snapshots, if is 1, it returns only TAPE files, and
	if is 2, it returns only MDR files */

struct fichero *read_directory(char *cpath,enum LOAD_FILE_TYPES kind) {

	struct fichero *listhead,*listend;
	struct dirent *entry;
	DIR *directory;
	struct stat estado;
	unsigned char path[MAX_PATH_LENGTH],fichero[MAX_PATH_LENGTH],extension[5],found;
	int bucle,length;

	strcpy(path,cpath);
	if('/'!=path[strlen(path)-1])
		strcat(path,"/"); // add the final / to the path

	listhead=malloc(sizeof(struct fichero));
	strcpy(listhead->nombre,"..");
	listhead->tipo=2;
	listhead->next=NULL;
	listend=listhead;

	directory=opendir(path);
	if(directory==NULL)
		return(listhead); // can't access the directory

	do {
		entry=readdir(directory);
		if((NULL!=entry)&&(strcmp(entry->d_name,"."))&&(strcmp(entry->d_name,".."))) {
			strcpy(fichero,path);
			strcat(fichero,entry->d_name);
			stat(fichero,&estado);
			found=0; // by default is not a valid file...
			length=strlen(entry->d_name);
			if(length>3) {
				extension[4]=0;
				for(bucle=0;bucle<4;bucle++)
					extension[bucle]=entry->d_name[length-4+bucle]; // copy the 4 last chars of the file (the extension)
				switch(kind) {
				case FILETYPE_S1S:
					if((!strcasecmp(extension,".s1s")))
						found=1; // is a .z80 or SNA file
//                  printf("%s-%d\n", fichero, found);
				break;
				case FILETYPE_TAP_SPC:
					if((!strcasecmp(extension,".tap"))||(!strcasecmp(extension,".spc")))
						found=1; // is a .tap file
				break;
				case FILETYPE_DISK:
					if(!strcasecmp(extension,".dsk"))
						found=1; // is a .mdr file
				break;
//				case FILETYPE_SCR:
//					if(!strcasecmp(extension,".scr"))
//						found=1; // is a .mdr file
//				break;
				default:
				break;
				}
			} else
				found=0;
			if(((found)||(S_ISDIR(estado.st_mode)))&&('.'!=entry->d_name[0]||strlen(entry->d_name)>1)) { // is a directory. We must add it
				listend->next=malloc(sizeof(struct fichero));
				listend=listend->next;
				listend->next=NULL;
				strcpy(listend->nombrepath,fichero);
				strcpy(listend->nombre,entry->d_name);
				if(S_ISDIR(estado.st_mode))
					listend->tipo=1; // a directory
				else
					listend->tipo=0; // a file
			}
		}
	} while(entry!=NULL);
	closedir(directory);
	return(listhead);
}

// deletes a filelist tree, freeing the memory used by it

void delete_filelist(struct fichero *filelist) {

	struct fichero *fl1,*fl2;

	fl1=fl2=filelist;

	while(fl1!=NULL) {
		fl2=fl1->next;
		free(fl1);
		fl1=fl2;
	}
}


/* allows user to choose a file from PATH. If KIND=0, only snapshots. If KIND=1, only
	 TAPE files */

#define MAX_LIST 12
char *select_file(char *path,enum LOAD_FILE_TYPES kind) {

	struct fichero *filelist,*fl2;
	unsigned char fin,read,*salida;
	int bucle,ancho,numitems,selected,from,longitud;

	salida=(unsigned char *)malloc(MAX_PATH_LENGTH);
	salida[0]=0;

	fin=1;
	read=1;
	selected=0;
	from=0;
	numitems=0;

	filelist=NULL;

	do {

		if(read) {
			filelist=read_directory(path,kind);
			read=0;

			fl2=filelist;
			numitems=0;
			while(fl2!=NULL) { // counts the number of items
				fl2=fl2->next;
				numitems++;
			}
			selected=0;
			from=0;
		}
        clean_screen();
		print_files(filelist,from,selected);

		switch(wait_key()) {
		case SDLK_ESCAPE: // to exit the help
			fin=0;
		break;
		case SDLK_UP:
			if(selected>0) {
				selected--;
				if(selected<from)
					from--;
			}
		break;
		case SDLK_DOWN:
			if(selected<(numitems-1)) {
				selected++;
				if(selected>(from+MAX_LIST)) // 23 is the total of items that can be displayed
					from++;
			}
		break;
		case SDLK_PAGEUP:
			for(bucle=0;bucle<15;bucle++)
				if(selected>0) {
					selected--;
					if(selected<from)
						from--;
				}
		break;
		case SDLK_PAGEDOWN:
			for(bucle=0;bucle<15;bucle++)
				if(selected<(numitems-1)) {
					selected++;
					if(selected>(from+MAX_LIST)) // 23 is the total of items that can be displayed
						from++;
				}
		break;
		case SDLK_RETURN:
			fl2=filelist;
			if(selected!=0)
				for(bucle=0;bucle<selected;bucle++)
					fl2=fl2->next;
			switch(fl2->tipo) {
			case 0: // select file
				strcpy(salida,fl2->nombrepath);
				delete_filelist(filelist);
				return(salida); // ends returning the filename
			break;
			case 1: // change directory
				strcpy(path,fl2->nombrepath); // new path_taps is namepath
				delete_filelist(filelist); // frees the memory
				read=1; // and redisplay all the files
			break;
			case 2: // upper directory
				longitud=strlen(path);
				if(longitud<2) // there's no upper directory
					break;
				if('/'==path[longitud-1]) { // is the char ended in '/' ?
					path[longitud-1]=0; // eliminated
					longitud--;
				}
				while('/'!=path[longitud-1]) {
					longitud--;
					path[longitud]=0;
				}
				if(longitud>2) { // it's not the upper directory
					longitud--;
					path[longitud]=0; // delete the final '/'
				}
				read=1;
			break;
			default:
			break;
			}
		break;
		default:
		break;
		}
	} while(fin);

	delete_filelist(filelist);
	return(NULL);

}

//void keyboard_menu() {
//
//	FILE *fichero;
//	int bucle1,bucle2,retval;
//	unsigned char *buffer,*buffer2,valor;
//
//	buffer=spcsdl.mscr->pixels;
//
//	clean_screen();
//	fichero=myfopen("fbzx/keymap.bmp","rb");
//	if (fichero==NULL) {
//		strcpy(spcsdl.osd_text,"Keymap picture not found");
//		spcsdl.osd_time=100;
//		return;
//	}
//	if (spcsdl.zaurus_mini==0) {
//		for (bucle1=0;bucle1<344;bucle1++)
//			for(bucle2=0;bucle2<640;bucle2++) {
//				retval=fscanf(fichero,"%c",&valor);
//				paint_one_pixel((unsigned char *)(colors+valor),buffer);
//				buffer+=spcsdl.bpp;
//			}
//	} else {
//		buffer+=(479*spcsdl.bpp);
//		for(bucle1=0;bucle1<344;bucle1++) {
//			buffer2=buffer;
//			for(bucle2=0;bucle2<640;bucle2++) {
//				retval=fscanf(fichero,"%c",&valor);
//				paint_one_pixel((unsigned char *)(colors+valor),buffer);
//				buffer+=(480*spcsdl.bpp);
//			}
//			buffer=buffer2-spcsdl.bpp;
//		}
//	}
//	//print_copy(spcsdl.mscr->pixels,spcsdl.mscr->w);
//	wait_key();
//	clean_screen();
//}

void clean_screen() {

//	int bucle;
//	unsigned char *buffer;
//
//	buffer=spcsdl.mscr->pixels;
//
//	for(bucle=0;bucle<((spcsdl.mscr->h)*(spcsdl.mscr->w)*(spcsdl.bpp));bucle++)
//		*(buffer++)=0;
	SDL_Rect rect;
	rect.x = rect.y = 0;
	rect.w = spcsdl.w*2; rect.h = spcsdl.h*2;
    SDL_FillRect(spcsdl.mscr, &rect, 0x0);
    SDL_FillRect(spcsdl.emul, &rect, 0x0);
}

// waits for a keystroke and returns its value

unsigned int wait_key() {

	char fin;
	unsigned int temporal_io=0;
	SDL_Event evento;

	fin=1;
	flip_screen(spcsdl.mscr);
	do {
		if(!SDL_WaitEvent(&evento))
			continue;

		if(evento.type!=SDL_KEYUP)
			continue;

		fin=0;

		temporal_io=(unsigned int)evento.key.keysym.sym;
	} while(fin);

	return (temporal_io);
}

// shows the files from the number FROM, and marks the file number MARK

void print_files(struct fichero *filelist,int from,int mark) {

	struct fichero *fl2;
	int bucle,numitems,ancho,pos;
	char ink1,ink2;
	unsigned char spaces[39]="                                      ";
	unsigned char namefile[MAX_PATH_LENGTH];

	fl2=filelist;
	numitems=0;

	while(fl2!=NULL) { // counts the number of items
		fl2=fl2->next;
		numitems++;
	}

	ink1=ink2=0;

	fl2=filelist;
	pos=72;
	for(bucle=0;bucle<numitems;bucle++) {
		if(bucle>=from) {
			strcpy(namefile,fl2->nombre);
			strcat(namefile,spaces);
			namefile[36]=0; // we print up to 36 chars
			switch(fl2->tipo) {
			case 0: // file
				ink1=15;
				ink2=14;
			break;
			case 1: // directory
				ink1=12;
				ink2=4;
			break;
			case 2: // parent directory
				ink1=10;
				ink2=2;
			break;
			}
			if(bucle==mark)
				print_string(spcsdl.mscr,namefile,-1,pos,ink2,15);
			else
				print_string(spcsdl.mscr,namefile,-1,pos,ink1,0);
			pos+=24;
		}
		if((pos+24)>192 * 2)
			break; // reached bottom part of the rectangle
		fl2=fl2->next;
	}
	while((pos+24<460)) {
		print_string(spcsdl.mscr,spaces,-1,pos,0,0);
		pos+=24;
	}
	flip_screen(spcsdl.mscr);
}
//
//void update_frequency (int freq)
//{
//if (freq == 0)
//		switch (spcsdl.mode128k) {
//	case 0:		// 48K
//		if (spcsdl.videosystem==0) spcsdl.cpufreq = 3500000;
//		else spcsdl.cpufreq = 3527500;
//	break;
//	case 1:		// 128K
//	case 2:		// +2
//	case 3:		// +2A/+3
//	case 4:		// spanish 128K
//		spcsdl.cpufreq = 3546900;
//	break;
//	default:
//	spcsdl.cpufreq = 3500000;
//	break;
//	}
//else spcsdl.cpufreq = freq*turbo_n;
//
//spcsdl.tst_sample=(spcsdl.cpufreq + (spcsdl.freq*N_SAMPLES/2))/(spcsdl.freq*N_SAMPLES);
//
//}
