#include <stdio.h>
#include "iocs.h"

unsigned char data[0x1001];

void cas2dsk();
void dsk2cas();
void dsk2dsk();

void main(void)
{
	uint8 c;
    printf("FDD check\n");
	sd_init();
    printf("FDD initialized\n");
	while(1)
	{
		//printf("Send State:%02x, Disk State:%02x\n", 0, sd_drvstate());
		printf("Disk Utility v0.1\n0. dsk2dsk\n1. cas2dsk\n2. dsk2cas\n3. format disk A\n4. format disk B\n?");
		c = getchar();
		printf("%c\n", c);
		switch (c)
		{
			case '0':
				dsk2dsk();
				break;
			case '1':
				cas2dsk();
				break;
			case '2':
				dsk2cas();
				break;
			case '3':
				sd_format(0);
				break;
			case '4':
				sd_format(1);
				break;
			case '5':
				return;
		}
	}
}
void dsk2dsk()
{
	unsigned char t;
	for(t = 0; t < 80; t++)
	{
		printf("FDD reading..track:%d\n", t);
		sd_read(16,0,t,1,data);
		printf("FDD writing..track:%d\n", t);
		sd_write(16,1,t,1,data);
	}
}
void cas2dsk()
{
	unsigned char t;
	for(t = 0; t < 80; t++)
	{
		printf("CAS loading..\n");
		cas_load(data, 0x1001);
		t = data[0x1000];
		printf("FDD writing..track:%d\n", t);
		sd_write(16,0,t,1,data);
	}
    printf("FDD transfer completed\n");
}

void dsk2cas()
{
	unsigned char t;
	for(t = 0; t < 80; t++)
	{
		printf("FDD reading..track:%d\n", t);
		sd_read(16,0,t,1,data);
		printf("CAS recording..track:%d\n", t);
		data[0x1000] = t;
		cas_save(data, 0x1001);
	}
    printf("CAS recording completed\n");
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