#include <stdio.h>
#include <string.h>
#include "iocs.h"

unsigned char data[0x1000];

void cas2dsk();
void dsk2cas();
void dsk2dsk();
uint8 listdir(uint8 p, uint8 c, uint8 pg);
void bootbasic();
int pifiles(char *);
int pioldnum();
void run(int);
void main(void)
{
	uint8 ch, p, l, c, pg, num;
	int t;
	char *files;
	const char *param = "SD:/\\*.tap";
	p = 0;
	c = 0;
	pg = 12;
	memset(data, 0, 0x800);
	cls();
	printf(" RPI extension box for SPC1000\n-------------------------------");
	ch = 0;
	t = pifiles(param);
	l = t / pg;
	num = pioldnum();
	p = num / pg;
	c = num % pg;
	while(1)
	{
		gotoxy(0,2);
		listdir(p, c, pg);
		printf("-------------------------------       RETURN for Execution");
		gotoxy(4,c+2);
		ch = getchar();
		switch (ch)
		{
			case 0x1d:
				p = (p > 0 ? p - 1: 0);
				break;
			case 0x1c:
				p = (p < l ? p + 1: l);
				break;
			case 0x1e:
				c = (c > 0 ? c - 1: 0);
				break;
			case 0x1f:
				c = (pg-1 > c ? c + 1: pg-1);
				break;
			case 0x0d:
				run((int)p * pg + c);
				break;
		}
	}
}

void run(int num)
{
	int k = num, s=0;
	cls2();
	while(k--) while(data[s++] != 0);
	gotoxy(10, 6);
	printf("Loading...");
	gotoxy((32-strlen(data+s))/2, 8);
	printf("%s", data+s);
	pload2(num);
}

uint8 listdir(uint8 p, uint8 c, uint8 pg)
{
	int i = pg * p;
	uint8 j = 0;
	int k = i;
	int s = 0;
	while(k--) while(data[s++] != 0);
	attr_clear();
	for(;j<pg;j++)
	{
		if (*(data+s) != 0)
			printf("%03d. %-25s\n", i+j, data+s);
		while(data[s++] != 0);
	}
	attr_set(1, 0x840+c*32, 32);
	return pg;
}
