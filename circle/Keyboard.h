#include "spckey.h"

#ifndef RESET
#define RESET()
#endif

class CKeyboard {
	TKeyMap spcKeyHash [0x200];
	unsigned char keyMatrix[16];
	static CKeyboard *s_pThis;
	bool pressed = false;
	int repeat = 0;	
public:
	static CKeyboard *GetInstance()
	{
		return s_pThis;
	}
	CKeyboard()
	{
		int num = 0;
		do {
			spcKeyHash[spcKeyMap[num].sym] = spcKeyMap[num];
		} while(spcKeyMap[num++].sym != 0);        
		s_pThis = this;
	}
	void clearMatrix() {
		for (int i = 0; i < sizeof(keyMatrix); i++)
			keyMatrix[i] = 0xff;
		pressed = false;
		// printf("cleared\n");
	}       	
	unsigned char matrix(char reg) {
		unsigned char ret = keyMatrix[(reg&0xf)];
		if (pressed && (reg&0xf) == 0)
			if (!repeat--)
				clearMatrix();
		return ret;
	}	
	void keyHandler(unsigned char ucModifiers, const unsigned char RawKeys[6])
	{
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
