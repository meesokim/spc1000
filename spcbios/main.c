#include <stdio.h>
#include "iocs.h"

unsigned char data[0x1001];

void cas2dsk();
void dsk2cas();
void dsk2dsk();
uint8 listdir(uint8 p, uint8 c, uint8 pg);
void pload(uint8 prg);
void main(void)
{
	uint8 ch, p, l, c;
	p = 0;
	cls();
	printf("RPI extension box for SPC1000\n");
	printf("------------------------------");
	while(1)
	{
		//printf("Send State:%02x, Disk State:%02x\n", 0, sd_drvstate());
		gotoxy(0,2);
		l = listdir(p, c, 10);
		ch = getch();
		switch (ch)
		{
			case KEY_F1:
				p = (p > 0 ? p - 1: 0);
				break;
			case KEY_F4:
				p = (l > 10 ? p + 1: l);
				break;
			case KEY_F2:
				c = (c > 0 ? c - 1: 0);
				break;
			case KEY_F3:
				c = (l > c ? c + 1: l);
				c = (c > 10 ? 10 : c);
				break;
			case KEY_F5:
				pload(p * 10 + c);
				break;
		}
	}
}

void dir(char *pp)
{
	unsigned char i = 1;
	char *p = pp;
//	= (FInfo *)pp;
	while(*p != 0 || p < pp + 256)
	{
		printf("%d.%s (%d)\n", i, p, *(p+15)*256);
		p += 1;
	}	
}

uint8 listdir(uint8 p, uint8 c, uint8 pg)
{
	
}
void pload(uint8 prg)
{
	
}
