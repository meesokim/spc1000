#include "keyboard.h"
// #include "spckey.h"
// #include "cassette.h"

extern Uint32 SDL_GetTicks(void);


CKeyboard::CKeyboard()
{
	BuildKeyHashTab();
}


/**
 * Build Keyboard Hashing Table
 * Call this once at the initialization phase.
 */
void CKeyboard::BuildKeyHashTab(void)
{
	int i;
	static int hashPos[256] = { 0 };

	for (i = 0; spcKeyMap[i].keyMatIdx != -1; i++)
	{
		int index = spcKeyMap[i].sym % 256;

		KeyHashTab[index].numEntry++;
		hashPos[index]++;
	}

	for (i = 0; i < 256; i++)
	{
		KeyHashTab[i].keys
			= (TKeyMap *) malloc(sizeof(TKeyMap) * hashPos[i]);
	}

	for (i = 0; spcKeyMap[i].keyMatIdx != -1; i++)
	{
		int index = spcKeyMap[i].sym % 256;

		hashPos[index]--;
		if (hashPos[index] < 0)
			printf("Fatal: out of range in %s:BuildKeyHashTab().\n",
			__FILE__), exit(1);
		KeyHashTab[index].keys[hashPos[index]]
			= spcKeyMap[i];
	}
}

/**
 * SDL Key-Down processing. Search Hash table and set appropriate keyboard matrix.
 * @param sym SDL key symbol
 */
void CKeyboard::ProcessKeyDown(SDL_Keycode sym)
{
	int i;
	int index = sym % 256;
    //printf(">%d-%c\n", sym);
	for (i = 0; i < KeyHashTab[index].numEntry; i++)
	{
		if (KeyHashTab[index].keys[i].sym == sym)
		{
			keyMatrix[KeyHashTab[index].keys[i].keyMatIdx]
				&= ~(KeyHashTab[index].keys[i].keyMask);
#ifdef DEBUG_MODE
			printf("%08x [%s] key down\n",
				KeyHashTab[index].keys[i].sym, KeyHashTab[index].keys[i].keyName);
#endif
			break;
		}
	}
}

/**
 * SDL Key-Up processing. Search Hash table and set appropriate keyboard matrix.
 * @param sym SDL key symbol
 */
void CKeyboard::ProcessKeyUp(SDL_Keycode sym)
{
	int i;
	int index = sym % 256;
    //printf("<%d-%c\n", sym);
	for (i = 0; i < KeyHashTab[index].numEntry; i++)
	{
		if (KeyHashTab[index].keys[i].sym == sym)
		{
			keyMatrix[KeyHashTab[index].keys[i].keyMatIdx]
				|= (KeyHashTab[index].keys[i].keyMask);
#ifdef DEBUG_MODE
			printf("%08x [%s] key up\n",
				KeyHashTab[index].keys[i].sym, KeyHashTab[index].keys[i].keyName);
#endif
			break;
		}
	}
}


/**
 * SDL Key-Down processing. Special Keys only for Emulator
 * @param sym SDL key symbol
 */
void CKeyboard::ProcessSpecialKey(SDL_Keysym ksym)
{
// 	int index = ksym.sym % 256;
// 	int retVal;
// 	FILE *rfp_save;
// 	FILE *wfp_save;
// 	char *str;
//     int r = 0;
//     int t;
// 	switch (ksym.sym)
// 	{
// 	case SDLK_SCROLLLOCK: // turbo mode
// 		TURBO = (TURBO ? 0: 10);  // toggle
// 		if (!TURBO) t = timeGetTime();
// 		printf("turbo %s\n", (TURBO)? "on":"off");
// 		break;
//     // case SDLK_INSERT:
//     //     if (m_uiStr == NULL)
//     //     {
//     //         // str = GetClipboardText(64000);
//     //         // if (str)
//     //         // {
//     //         //     printf("clipboard:%s\n",str);
//     //         //     UI_Paste(str);
//     //         // }
//     //     }
//     //     break;
// 	case SDLK_PRINTSCREEN:
//     case SDLK_SYSREQ:
//         // retVal = SetClipboardText((const char *)spcsys.prt.bufs);
// 		// PRT_Save((const char *)spcsys.prt.bufs, spcsys.prt.length);
//         // printf("Printer Output.(%d)\n%s\n", retVal, spcsys.prt.bufs);
//         break;

// //     case SDLK_F6:
// // 	    if (ksym.mod & KMOD_ALT)
// //         {
// //             snapshots_menu();
// //         } else {
// //             help_menu();
// //         }
// //         break;
// //     case SDLK_F7:
// // 	    if (ksym.mod & KMOD_ALT)
// //         {
// //             settings_menu();
// //         } else {
// //             taps_menu();
// //         }
// //         break;
// // 	case SDLK_F8: // PLAY button
// // 	    printf("SDLK_F8 pressed\n");
// // 		if (spconf.rfp != NULL)
// // 			FCLOSE(spconf.rfp);
// // 		if (ksym.mod & KMOD_ALT) // STOP button
// //         {
// //             spcsys.cas.button = CAS_STOP;
// //             spcsys.cas.motor = 0;
// //             printf("stop button\n");
// //         }
// //         else // PLAY button
// //         {
// // 			if (spconf.wfp != NULL)
// // 			{
// // 				spcsys.cas.button = CAS_REC;
// // 				spcsys.cas.motor = 1;
// // 				printf("rec button pushed\n");
// //  			}
// // 			else if (OpenTapeFile() < 0)
// //                 break;
// // 			else {
// // 				spcsys.cas.button = CAS_PLAY;
// // 				spcsys.cas.motor = 1;
// // 			}
// //             spcsys.cas.lastTime = 0;
// //             ResetCassette(&spcsys.cas);
// //             printf("play button pushed\n");
// //         }
// // 		break;
// // 	case SDLK_F9: // FDD management
// // 		if (ksym.mod & KMOD_ALT)
// // 		{
// // 			VDP_Save();
// // 		}
// // 		else
// // 		{
// // 			printf("Floppy Disk management\n");
// // 			floppy_disk_menu();
// // 		}
// // 		break;
// // 	case SDLK_F10: // Quit
// //         SDL_Quit();
// //         exit(0);
// //         break;
// // 	case SDLK_PAGEUP: // Image Save        puts("q          : Exit Z80 emulation");
// // 		save_s1sfile();
// // 		printf("Image Save\n");
// // 		break;

// // 	case SDLK_PAGEDOWN: // Image Load
// // 		load_s1sfile();
// // //		r = 1;
// // //		spcsys.tick = SDL_GetTicks();
// // //		spcsim.baseTick = SDL_GetTicks();
// // //		spcsim.prevTick = spcsim.baseTick;
// // //		spcsys.cas.button = CAS_STOP;
// // //		spcsys.cas.motor = 0;
// // //		if (spcsys.GMODE & 0x08)
// // //		{
// // //			SetMC6847Mode(SET_GRAPHIC, spcsys.GMODE);
// // ////			UpdateMC6847Gr(MC6847_UPDATEALL);
// // //		}
// // //		else
// // //		{
// // //			SetMC6847Mode(SET_TEXTMODE, spcsys.GMODE);
// // ////			UpdateMC6847Text(MC6847_UPDATEALL);
// // //		}
// // //		printf("Image Load\n");
// // 		break;

// 	case SDLK_F11: // PC Keyboard mode, thanks to zanny
// 	    if (ksym.mod & KMOD_ALT)
//         {
//             // SetPCKeyboard(spcsys.RAM);
//         }
//         else
//         {
//             ToggleFullScreen();
//         }
// 		break;
//     case SDLK_KP_5:
//         spconf.debug = 1;
//         break;
// 	case SDLK_F12: // Reset
//         if (ksym.mod & KMOD_ALT)
//         {
//             spcsys.IPL_SW = 1;
//             printf("Reset with IPL_SW\n");
//         }
//         else
//         {
//             spcsys.IPL_SW = 0;
//             printf("Reset (keeping tape pos.)\n");
//         }
// 		// load_rom();
// 		// InitIOSpace();
// 		SndQueueInit();
// //		SetMC6847Mode(SET_TEXTMODE, 0);
//         spcsys.Z80R.cyc = I_PERIOD;
//         spcsys.Z80R.pc = 0x00;
//         //spcsys.Z80R.SP.W = 0xf000;
//         spcsys.IPLK = 1;
//         spcsys.IPL_SW = 1;
//         // z80mem = spcsys.ROM;
// 		spcsim.baseTick = SDL_GetTicks();
// 		spcsim.prevTick = spcsim.baseTick;
// 		spcsys.intrTime = INTR_PERIOD;
// 		spcsys.tick = 0;
//         spcsys.refrTimer = 0;	// timer for screen refresh
//         spcsys.refrSet = spconf.frameRate;	// init value for screen refresh timer
// 		z80_init(&spcsys.Z80R);
// 		break;
// 	}
}
