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
	word protct;
} HEADER;

char strict;

int dump();
int fgetc2(FILE *f);
int error = 0;
int num = 0;
int split = 0;
int fpos = 0;
int bin = 0;
int reverse = 0;
int cas = 0;
char binfilename[256];
HEADER *head;
char prefix[256];

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
            {
                fprintf(OUT, b[i] & (1<<j) ? "1" : "0");
            }
			fprintf(OUT, "1");
		}
		for(int i = 0; i < 20; i++)
			fprintf(OUT, "0");
	}
}

void binfile(byte *b, int len)
{
	char filename[1024];
	sprintf(filename, "%s-%d.bin", binfilename, bin++);
	printf("%s ... created\n", filename);
	FILE *f = fopen(filename, "wb");
	if (f)
	{
		fwrite(b, 1, len, f); 
		fclose(f);
	}
}

void getfilename(char *fname, char *title)
{
	char c;
    strcpy(fname, title);
	for(int i = strlen(fname)-1; i >= 0; i--)
	{
		c = title[i];
        // if (c == ' ' && endspace == 0)
        // {
            // fname[i] = 0;
            // continue;
        // }
        // endspace = 1;
		if (c == '\"')
			c = '\'';
		else if (c == '>')
			c = '_';
		else if (c == '=')
			c = '_';
		fname[i] = c <= '~' && c >= ' ' ? c : '_'; 
	}
}

int getByte(FILE *in)
{
	int v, i;
	char c;
	v = 0;
	for(i = 0; i < 8; i++) {
		if (!reverse)
			v += (fgetc2(in) == '1' ? 1 << (7-i) : 0);
		else
			v += (fgetc2(in) == '1' ? 1 << i : 0);

	}
	c = fgetc2(in);
	if (c == '1')
		error = 0;
	else
	{
		if (c == '#')
			error = 2;
		else 
			error = 1;
	}
		// if (strict) {
			// printf("\x1b[31m%02x\x1b[0m=>check bit error(%d)!\n", v, fpos);
	    	// exit(0);
		// }
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
	char *point;
	IN = 0;
    prefix[0] = 0;
	if (argc < 1) {
		IN = stdin;
		if (getchar() < 0)
		{
			printf ("usage: %s tap_filename\n", argv[0]);
			return 1;
		}
	} else if (argc > 2) {
		pos = atoi(argv[2]);
        for(int i = 2; i < argc; i++)
        {
			if (strlen(argv[i]) > 2)
				continue;
            switch (*argv[i])
            {
                case '-':
                    split = 1;
                    strcpy(prefix, argv[i]+1);
                    break;
                case 'b':
                    bin = 1;
                    break;
                case 'r':
                    reverse = 1;
                    break;
                case 'R':
                    reverse = 1;
                    split = 1;
                    break;
            };
        }
	} 
	strict = 1;
	printf("argc:%d\n", argc);
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			strcpy(binfilename, argv[i]);
			IN = fopen(argv[i], "rb");
			if (!IN)
				continue;
			printf("FILE:%d.%s\n", i, binfilename);
			if((point = strrchr(argv[i],'.')) != NULL ) {
				if(strcmp(point,".cas") == 0) {
					cas = 1;
					printf("cas = 1\n");
				}
			}
			length = 0;
			while (length > -1)
			{
				length = dump(length);
				//printf("length=%d\n", length);
				// if (length < 0)
				// 	break;
			}
			fclose(IN);
		}
		exit(0);
	}
    else
    {
        IN = stdin;
    }
	if (pos != 0) {
		fseek(IN, pos, SEEK_SET);
	}
	if (!IN) {
	    printf("Could not open file %s for reading.\n", argv[1]);
		return 2;
	}//if
	fseek(IN, 0, SEEK_SET);
	while (1)
	{
		length = dump(length);
		//printf("length=%d\n", length);
		if (length < 0)
			break;
	}	
	if (TAP)
		fclose(TAP);
}

int fgetc2(FILE *f)
{
	int i;
	static char pos = 0;
	static unsigned char value = 0;
	if (cas)
	{
		if (pos == 0)
		{
			value = fgetc(f);
		}
		i = (value >> (7-pos++)) & 1 ? '1' : '0';
		if (pos > 7)
			pos = 0;
	}
	else
	{
		do {
			i = fgetc(f);
			fpos++;
			//printf("%c", i);
			if (i == '|')
				i = '1';
			else if (i == '@')
				i = '0';
			if (feof(f))
				break;
		} while (i != '0' && i != '1' && i != '#');
	}
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
		{	
			if (c1 > 18 && c0 > 18)
				break;
			else
				c1 = c2 = c0 = 0;
		}
		cc = c;
			if (c2 == 2)	printf("c0=%d,c1=%d,c2=%d\n", c0, c1, c2);
	}
	//printf("position=%ld\n", ftell(IN));
	if (fgetc2(IN) == EOF)
		return -1;
	return 0;
}

char* replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    while (current_pos){
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}

int dump(int len) {
	char filename[1024];
	char name[16];
	char *escapes[3] = {"%02x ", "\x1b[31m%02x\x1b[0m*",  "\x1b[1;31m%02x\xb[0m#"};
	int d = 0, v = 0;
	int csum = 0;
	int csum1 = 0;
	int length = 0;
    int unknown = 0;
	int pos = ftell(IN);
	if (tag() != 0)
		return -1;
	if (len == 0)
		len = 128;
	while(len-->0 || unknown == 1) {
		v = getByte(IN);
        if (!unknown || !error)
            csum += getChecksum(v);
		b[d++] = v;
        if (len == 127 && d == 1 && v > 2)
        {
            unknown = 1;
            len = 10000000;
        }
#if 1
		if (length % 16 == 0)
			printf("\n%04x:", length);
		length++;
		// if (error == 1 && unknown == 1)
		// {
        //     fseek(IN, -18, SEEK_CUR);
        //     csum -= getChecksum(b[d-3])+getChecksum(b[d-2]);
        //     length -= 3;
        //     csum1 = (b[d-3] << 8) + b[d-2];
        //     break;
		// }
		// printf(escapes[error%3], v);
		printf(escapes[error], v);
#endif		
	}
    if (!error)
    {
        csum1 = getByte(IN) << 8;
        csum1 += getByte(IN);
    }
	if (reverse)
	{
		csum1 = ~csum1;
		csum1 &= 0xffff;
	}
	csum = csum & 0xffff;
	if (d == 128) 
	{
		head = (HEADER *) b;
		head->name[16] = 0;
		printf("\n\nName: %s\n", head->name);
		#if 0
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
		#endif
		printf("Type: %s\n", head->type == 1 ? "Machine(1)" : "Basic(0)");
		printf("Length: %04xh\n", head->size);
		printf("Loading address: %04xh\n", head->load);
		printf("Checksum: %04xh (%04xh calculated, %s)\n", csum1, csum, (csum1 == csum ? "matched" : "mismatched"));
		printf("Header Size: %d\n", d);
		if (head->type == 1)
			printf("Jump address: %04xh\n", head->jump);
		printf("Protect: %04x\n", head->protct);
		printf("File Position: %d\n\n", pos);
		getfilename(name, head->name); 
		if (split)
		{
			if (TAP)
				fclose(TAP);
            if (*prefix)
            {
                sprintf(filename, "%s_%d_%s.tap", prefix, ++num, name);
            }
            else
            {
                sprintf(filename, "%d_%s.tap", ++num, name);
            }
			TAP = fopen(filename, "w+");
			printf ("file:%s (%x)\n",filename, csum);
			writefile(TAP, b, d, csum);
		}
		return head->size;
	}
	else
	{
		printf("\nBody Summary\n");
		printf("Length: %d\n", d-1);
		printf("Checksum: %04xh (%04xh calculated, %s)\n", csum1, csum, (csum1 == csum ? "matched" : "mismatched"));
//		printf("bin: %d\n", bin);
		if (split && TAP)
			writefile(TAP, b, d, csum);
		if (bin)
			binfile(b, d);
	}
	return 0;
}
