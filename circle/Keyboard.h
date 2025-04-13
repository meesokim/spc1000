#include "spckey.h"

#ifndef RESET
#define RESET()
#endif

class CKeyboard {
	TKeyMap spcKeyHash [0x200];
	unsigned char keyMatrix[16];
public:
	CKeyboard()
	{
		int num = 0;
		do {
			spcKeyHash[spcKeyMap[num].sym] = spcKeyMap[num];
		} while(spcKeyMap[num++].sym != 0);        
	}
	void KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6]) 
	{
		CString Message;
		Message.Format ("Key status (modifiers %02X, %s)", (unsigned) ucModifiers, RawKeys);
		TKeyMap *map;
		for(int i = 0; i < 10; i++)
			keyMatrix[i] = 0xff;
		if (ucModifiers != 0)
		{
			if ((ucModifiers & 0x10 || ucModifiers & 0x01) & (ucModifiers & 0x40 || ucModifiers & 0x4)) {
				if (RawKeys[0] == 0x4c)
					RESET();
			}
			for(int i = 0; i < 8; i++)
				if ((ucModifiers & (1 << i)) != 0)
				{
					map = &spcKeyHash[0x100 | (1 << i)];
					if (map != 0)
						keyMatrix[map->keyMatIdx] &= ~ map->keyMask;
				}
		}
	
		for (unsigned i = 0; i < 6; i++)
		{
			if (RawKeys[i] != 0)
			{
				map = &spcKeyHash[RawKeys[i]];
				if (map != 0)
					keyMatrix[map->keyMatIdx] &= ~ map->keyMask;
			}
		}		
	}
};
