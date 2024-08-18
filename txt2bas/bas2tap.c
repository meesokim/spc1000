/*
 *	Quick 'n' dirty mym to tap converter
 *
 *	Usage: bin2tap [binfile] [tapfile]
 *         or: bin2tap [binfile] [tapfile] [start]
 *         or: bin2tap [binfile] [tapfile] [start] [execution]
 *
 *	zack 8/2/2000
 *	Modified by Stefano	3/12/2000
 *      Modified by Metalbrain 27/06/2002
 *
 *	Creates a new TAP file (overwriting if necessary) just ready to run.
 */

#include <stdio.h>
#include <string.h>
/* Stefano reckons stdlib.h is needed for binary files for Win compilers*/
#include <stdlib.h>

typedef unsigned short word;
typedef unsigned char byte;

typedef struct  {
	byte type;
	char name[17];
	word size;
	word load;
	word jump;
	byte protct;
	char dummy[126-25];
	word checksum;
} HEADER;

#define HEADn sizeof(HEADER)

unsigned char parity;

void writebyte(unsigned char, FILE *);
void writebytes(unsigned char, int, FILE *);
int writetaps(unsigned char, FILE *);
void writechecksum(int, FILE *);
void parseIHEX(FILE *);

unsigned char binary[65536];
//void writenumber(unsigned int, FILE *);
//void writeword(unsigned int, FILE *);
word getChecksum(FILE *);
int exeat, datastart;
int	len;

int main(int argc, char *argv[])
{
	char	name[17];
	char    tapfile[256];
	FILE	*fpin, *fpout;
	int	c;
	int	i,j;
	int cs = 0;
	int dotpos = 0, slashpos = 0;
	HEADER *head;
	char block[126];
	head = (HEADER *) &block[0];
	memset(head, 0, sizeof(HEADER));
	if ( (argc<2) || (argc>5) ) {
		fprintf(stdout,"Usage: %s [cas file]\n",argv[0]);
		fprintf(stdout,"   or: %s [cas file] [tap file]\n",argv[0]);
		exit(1);
	}

	if (argc < 3)
	{
		strcpy(tapfile, argv[1]);
		for(i = strlen(argv[1]); i > 0; i--)
		{
			c = *(tapfile+i);
			if (c == '.' && dotpos == 0)
				dotpos = i;
			if ((c == '\\' || c == '/') && slashpos == 0)
				slashpos = i;
		}
		if (dotpos > 0)
		{
			strcpy(tapfile+dotpos+1, "tap");
			if (slashpos > 0)
				strcpy(tapfile, tapfile+slashpos+1);
		}
		else
			strcpy(tapfile+strlen(argv[1]), "tap");
	}
	else
		strcpy(tapfile, argv[2]);
		
	strcpy(name, argv[1]);

	datastart=0x7c9d;
	exeat=0;

	printf("datastart:%04x\n", datastart);
	if ( (fpin=fopen(argv[1],"rb") ) == NULL ) {
        fprintf(stdout,"Can't open input file\n");
		exit(1);
	}
	if (strncasecmp(argv[1] + strlen(argv[1]) - 3, "ihx", 3) == 0 || strncasecmp(argv[1] + strlen(argv[1]) - 4, "ihex", 4) == 0)
	{
		printf("tetstset\n");
		parseIHEX(fpin);
	}
	else
	{
		i = 0;
		while((c = getc(fpin)) != EOF)
		{
			binary[datastart + i++] = c;
		}
		len = i;
	}

/*
 *	Now we try to determine the size of the file
 *	to be converted
 */
	
	fseek(fpin,0L,SEEK_SET);
	
	head->type = 2;
	if (strlen(name) > 17)
		strncpy(head->name, name+strlen(name)-17, 16);
	else
		strcpy(head->name, name);
	head->size = len;
	head->load = datastart;
	head->jump = exeat;
	head->protct = 0x3a;

	printf("start=%04x,exec=%04x,len=%04x\n", datastart, exeat, len);
	if ( (fpout=fopen(tapfile,"wb") ) == NULL ) {
		printf("Can't open output file\n");
		exit(1);
	}

/* Write a header, first */

	writebytes('0',100,fpout);
	writebytes('1',40,fpout);
	writebytes('0',40,fpout);
	writebyte('1',fpout);
	writebyte('1',fpout);
	for(i=0;i<HEADn;i++)
	{
		cs += writetaps(block[i], fpout);
		writebyte('1', fpout);
	}
 	writechecksum(cs, fpout);
	writebytes('0',400,fpout);
	writebytes('1',20,fpout);
	writebytes('0',20,fpout);
	writebyte('1',fpout);
	writebyte('1',fpout);

	cs = 0;
	for(i=0;i<len;i++)
	{
		c = binary[datastart + i];
		cs += writetaps(c, fpout);
		writebyte('1', fpout);
	}
	writechecksum(cs, fpout);
	writebytes('0',10, fpout);

	fclose(fpin);
	fclose(fpout);
}

word getChecksum(FILE *fp)
{
	int csum = 0;
	int b, j;
	while(b = getc(fp) != EOF) {
		for(j = 0; j < 8; j++)
			csum += (b>>j & 1);
	}
	return csum;
}

void writechecksum(int i, FILE *fp)
{
	writetaps(i/256,fp);
	writebyte('1', fp);
	writetaps(i%256,fp);
	writebyte('1', fp);
}

int writetaps(unsigned char c, FILE *fp)
{
	int j;
	int csum = 0;
	for(j=7;j>=0;j--)
	{
		if ((c>>j) & 1)
		{
			writebyte('1', fp);
			csum++;
		}
		else
			writebyte('0', fp);
	}
	return csum;
}

void writebyte(unsigned char c, FILE *fp)
{
	fputc(c,fp);
}

void writebytes(unsigned char c, int len, FILE *fp)
{
	int i;
	for(i=0; i<len; i++)
		fputc(c,fp);
}

int atoh(const char *str)
{
    int Value = 0, Digit;
    char c;

    while ((c = *str++) != 0) {

        if (c >= '0' && c <= '9')
            Digit = (int) (c - '0');
        else if (c >= 'a' && c <= 'f')
            Digit = (int) (c - 'a') + 10;
        else if (c >= 'A' && c <= 'F')
            Digit = (int) (c - 'A') + 10;
        else
            return -1;

        Value = (Value << 4) + Digit;
    }

    return Value;
}

int atoh2(const char *str, int f, int e)
{
    int Value = 0, Digit;
    char c;
	const char *str2 = str + f;
    while (e--) {

		c = *(str2++);
		if (c == 0)
			break;
        if (c >= '0' && c <= '9')
            Digit = (int) (c - '0');
        else if (c >= 'a' && c <= 'f')
            Digit = (int) (c - 'a') + 10;
        else if (c >= 'A' && c <= 'F')
            Digit = (int) (c - 'A') + 10;
        else
            break;

        Value = (Value << 4) + Digit;
    }

    return Value;
}


void parseIHEX(FILE *fp)
{
	char str[2048];
	int i, count, type, addr = 0xffff, cs, dataend = 0;
	len = 0;
	datastart = addr;
	while((cs = fscanf(fp, "%s", str)) != EOF)
	{
		if (str[0] == ':')
		{
			cs = 0;
			count = atoh2(str, 1, 2);
			len += count;
			cs += count;
			addr = atoh2(str, 3, 4);
			cs += atoh2(str, 3, 2) + atoh2(str, 5, 2);
			type  = atoh2(str, 7, 2);
			cs += type;
//			printf("%02x,%04x,%02x\n", count, addr, type);
			switch (type)
			{
				case 0:
					if (datastart > addr)
					{
//					printf("datastart=%04x, addr=%04x\n", datastart, addr);
						datastart = addr;
					}
					for(i=0;i<count;i++)
					{
						binary[addr+i] = atoh2(str, 9 + (i*2), 2);
//						printf("%02x,",binary[addr+i]);
						cs += binary[addr+i];
					}
					if (addr+i > dataend)
						dataend = addr+i;
/* 					if (0xff - (cs & 0xff) + 1 != atoh2(str, strlen(str)-2,2))
					{
						printf("Checksum error: %s (expect=%02x, calculated=%02x, sum=%x)\n", str, atoh2(str, strlen(str)-2,2), 0xff - (cs & 0xff), cs );
						exit(-1);
					}
 */					break;
				case 1:
					// eof
//					printf("EOF\n", cs);
//					printf("start=%04x,end=%04x,len=%04x\n", datastart, dataend, dataend-datastart);
					len = dataend - datastart;
					exeat = datastart;
					return;
			}
//			printf("\n");
		}
	}
}
 