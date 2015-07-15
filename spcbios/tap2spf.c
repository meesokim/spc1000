#include <stdio.h>
#include <stdlib.h>
#include "console.h"

FILE *IN = NULL;

word offset = 0x0,
     fs = 0x0,
     la = 0x0,
     ea = 0x0;
	 
static byte b[0xffff];
typedef struct  {
	byte type;
	char name[17];
	word size;
	word load;
	word jump;
} HEADER;

int dump();

int getByte(FILE *in)
{
	int v, i;
	v = 0;
	for(i = 0; i < 8; i++) {
		v += (fgetc2(in) == '1' ? 1 << (7-i) : 0);
	}
	return v;
}

int getChecksum(int v)
{
	int csum = 0, i = 0;
	for(i = 0; i < 8; i++) {
		csum += (v >> i & 1);
	}
	return csum;
}

int main(int argc, char **argv) {

	int length = 0;
	if (argc < 2) {
		printf ("usage: %s tap_filename\n", argv[0]);
		return 1;
	}
	
	IN = fopen(argv[1], "rb");
	if (!IN) {
	    printf("Could not open file %s for reading.\n", argv[1]);
		return 2;
	}//if
	
	while (1)
	{
		length = dump(length);
		if (length < 0)
			break;
	}	
}

int fgetc2(FILE *f)
{
	int i = fgetc(f);
	//printf("%c", i);
	return i;
}

int tag() {
	int c = 0, cc = 0, c0 = 0, c1 = 0, c2 = 0;
	while((c = fgetc2(IN)) >= 0) {
		if (c == EOF)
			return -1;
		c1 += (cc == c && c == '1');
		c0 += (cc == c && c == '0');
		if (cc == '0' && c == '1')
			if (c1 > 18 && c0 > 18)
				break;
			else
				c1 = c2 = c0 = 0;
		cc = c;
//		if (c2 == 2)	printf("c0=%d,c1=%d,c2=%d\n", c0, c1, c2);
	}
	if (fgetc2(IN) == EOF)
		return -1;
	return 0;
}
int dump(int len) {
	HEADER *head;
	int i = 0, j = 0, d = 0, v = 0, c = 0, c0 = 0, c1 = 0;
	int csum = 0;
	int csum1 = 0;
	int length = 0;
	if (tag() != 0)
		return -1;
	if (len == 0)
		len = 128;
	while(1) {
		csum += (c == '1');
		v = getByte(IN);
		csum += getChecksum(v);
		if (fgetc(IN) != '1')	break;
		b[d++] = v;
		if (len <= 0)
			break;
		else
			len--;
		if (length % 16 == 0)
			printf("\n%04x:", length);
		length++;
		printf("%02x ", v);
	}
	csum1 += getByte(IN);
	fgetc(IN);
	csum1 += getByte(IN) << 8;
	fgetc(IN);
	printf("\n\n", d);
	if (d == 129) 
	{
		head = (HEADER *) b;
		printf("Name: %s\n", head->name);
		printf("Type: %s\n", head->type == 1 ? "Machine" : "Basic");
		printf("Length: %04xh\n", head->size);
		printf("Loading address: %04xh\n", head->load);
		printf("Checksum: %04xh (%04xh)\n", csum1, csum);
		printf("Header Size: %d\n", d-1);
		if (head->type == 1)
			printf("Jump address: %04xh\n", head->jump);
		return head->size;
	}
	else
	{
		printf("Body Loading..\n");
		printf("Length: %d\n", d-1);
		printf("Checksum: %04xh (%04xh)\n", csum1, csum%65536);		
	}
	return 0;
}