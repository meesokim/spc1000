/*
	wav2tap tool by F.Frances
	version 1.1: fixed a difference between wav2tap's behavior and
	the real Oric: the Oric accepts non-$16 values after the three
	$16 bytes (four when counting the first synchronized byte) and
	before the starting $24 byte. Thanks to Simon for spotting this.
	
	wav2tap for spc-1000 by meeso.kim
	version 1.2: revised for spc-1000 wav file.
*/


#include <stdio.h>

void exit(int);

struct {
	char sig[4];
	int riff_size;
	char datasig[4];
	char fmtsig[4];
	int fmtsize;
	short tag;
	short channels;
	int freq;
	int bytes_per_sec;
	short byte_per_sample;
	short bits_per_sample;
	char samplesig[4];
	int length;
} sample_riff;

int sync_ok=0;
int offset,pos;
FILE *in, *out;

main(int argc,char **argv)
{	
	unsigned start,end,byte;
	if (argc!=3) { printf("Wav2tap special Simon\nUsage: wav2tap file.wav file.tap\n",argv[0]); exit(1);}
	in=fopen(argv[1],"rb");
	if (in==NULL) { printf("Unable to open WAV file\n"); exit(1);}
	fread(&sample_riff,sizeof(sample_riff),1,in);
//	if (sample_riff.channels!=1 || sample_riff.freq!=4800 || sample_riff.byte_per_sample!=1) {
//		printf("Invalid WAV format: should be 4800 Hz, 8-bit, mono\n");
//		exit(1);
//	}
	printf("Channels:%d, Freq=%d, Sampling=%d\n", sample_riff.channels, sample_riff.freq, sample_riff.byte_per_sample);	
	out=fopen(argv[2],"wb");
	if (out==NULL) { printf("Unable to create TAP file\n"); exit(1);}

	for (;;) {
//		putc(getbit());
		
		fprintf(out,"%d", getbit());
		printf("\n");
	}
}

int getbyte(FILE *f)
{
	unsigned char a;
	a = getc(f);
	return a;
}
int getshort(FILE *f)
{
	unsigned short a;
	a = getc(f) + getc(f) << 8;
	return a;
}
int getint(FILE *f)
{
	int a;
	fscanf(f, "%d", &a);
	return a;
}

int getc2(FILE *f)
{
	int val;
	switch (sample_riff.byte_per_sample)
	{
		case 1:
			val = getbyte(f);
			break;
		case 2:
			val = getshort(f);
			break;
		case 4:
			val = getbyte(f);
			break;
	}
	if (feof(f)) exit(0);
	printf("%d ", val);
	return val;
}

getbit()
{
	int val,length,min;
skip:
	length=1;
	val=getc2(in);
	
	while (val>=0) {
		length++;
		val=getc2(in);
	}
	length++;
	val=getc2(in);
	while (val<=0) {
		length++;
		val=getc2(in);
	}
	if (length>30*sample_riff.freq/4800) return 1;
	else return 0;
}