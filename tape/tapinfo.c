#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "console.h"

FILE *IN = NULL;
FILE *TAP = NULL;

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

char strict;

int dump();
int fgetc2(FILE *f);
int error = 0;
int num = 0;

void writefile(FILE *OUT, byte *b, int len, int csum)
{
	b[len] = (csum >> 8) & 0xff;
	b[len+1] = csum & 0xff;
	if (OUT != NULL){
		if (len == 128)
			fprintf(OUT, "1111111111111111111111111111111111111111000000000000000000000000000000000000000011");
		else
			fprintf(OUT, "111111111111111111110000000000000000000011");
		for(int i = 0; i < len+2; i++) 
		{
			for(int j = 7; j >= 0; j--)
				if (b[i] & (1<<j))
					fprintf(OUT, "1");
				else
					fprintf(OUT, "0");
			fprintf(OUT, "1");
		}
		for(int i = 0; i < 200; i++)
			fprintf(OUT, "0");
	}
}

int getByte(FILE *in)
{
	int v, i;
	char c;
	v = 0;
	for(i = 0; i < 8; i++) {
		v += (fgetc2(in) == '1' ? 1 << (7-i) : 0);
	}
	c = fgetc2(in);
	if (c != '1' && c != '#')
	{
		//printf("%02x=>check bit error(%ld)!\n", v, ftell(in));
		if (c == '#')
			error = 2;
		else 
			error = 1;
		if (strict)
			exit(0);
	}
	else
		error = 0;
	if (feof(in))
		exit(0);
	return v;
}

int getChecksum(int v)
{
	int csum = 0, i = 0;
	for(i = 0; i < 8; i++) {
		csum += ((v >> i) & 1);
	}
	return csum;
}

int main(int argc, char **argv) {

	int length = 0;
	int pos = 0;
	if (argc < 2) {
		printf ("usage: %s tap_filename\n", argv[0]);
		return 1;
	} else if (argc == 3) {
		pos = atoi(argv[2]);
	}
	strict = 0;
	IN = fopen(argv[1], "rb");
	if (pos != 0) {
		fseek(IN, pos, SEEK_SET);
		
	}
	if (!IN) {
	    printf("Could not open file %s for reading.\n", argv[1]);
		return 2;
	}//if
	while (1)
	{
		length = dump(length);
		printf("length=%d\n", length);
		if (length < 0)
			break;
	}	
	if (TAP)
		fclose(TAP);
}

int fgetc2(FILE *f)
{
	int i;
	do {
		i = fgetc(f);
		//printf("%c", i);
		if (i == '|')
			i = '1';
		else if (i == '@')
			i = '0';
		if (feof(f))
			break;
	} while (i != '0' && i != '1' && i != '#');
	
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
		{	
			if (c1 > 18 && c0 > 18)
				break;
			else
				c1 = c2 = c0 = 0;
		}
		cc = c;
			if (c2 == 2)	printf("c0=%d,c1=%d,c2=%d\n", c0, c1, c2);
	}
	printf("position=%ld\n", ftell(IN));
	if (fgetc2(IN) == EOF)
		return -1;
	return 0;
}
int dump(int len) {
	HEADER *head;
	char filename[1024];
	char name[16];
	char c = 0;
	int d = 0, v = 0;
	int csum = 0;
	int csum1 = 0;
	int length = 0;
	if (tag() != 0)
		return -1;
	if (len == 0)
		len = 128;
	while(len-->0) {
		v = getByte(IN);
		csum += getChecksum(v);
		b[d++] = v;
#if 1
		if (length % 16 == 0)
			printf("\n%04x:", length);
		length++;
		printf("%02x%c", v, error == 0 ? ' ' : error == 1 ? '*' : '#' );
#endif		
	}
	csum1 += getByte(IN) << 8;
	csum1 += getByte(IN);
	if (d == 128) 
	{
		head = (HEADER *) b;
		head->name[16] = 0;
		printf("\n\nName: %s\n", head->name);
		for(int i = 0; i < 16; i++) 
		{
			printf("1");
			for(int j = 7; j >= 0; j--)
				if (head->name[i] & (1<<j))
					printf("1");
				else
					printf("0");
		}
		printf("\n");
		printf("Type: %s\n", head->type == 1 ? "Machine(1)" : "Basic(0)");
		printf("Length: %04xh\n", head->size);
		printf("Loading address: %04xh\n", head->load);
		printf("Checksum: %04xh (%04xh calcualted, %s)\n", csum1, csum, (csum1 == csum ? "matched" : "mismatched"));
		printf("Header Size: %d\n", d);
		if (head->type == 1)
			printf("Jump address: %04xh\n", head->jump);
		for(int i = 15; i >= 0; i--)
		{
			c = head->name[i];
			if (c == '\"')
				c = '\'';
			else if (c == '>')
				c = '_';
			else if (c == '=')
				c = '_';
			name[i] = c <= '~' && c >= ' ' ? c : 0; 
		}
		sprintf(filename, "%d_%s.tap", ++num, name);
		if (TAP)
			fclose(TAP);
		TAP = fopen(filename, "w+");
		printf ("file:%s=%x\n",filename, (int)TAP);
		writefile(TAP, b, d, csum);
		return head->size;
	}
	else
	{
		printf("\n\nBody Summary\n");
		printf("Length: %d\n", d-1);
		printf("Checksum: %04xh (%04xh)\n", csum1, csum);		
		writefile(TAP, b, d, csum);
	}
	return 0;
}
