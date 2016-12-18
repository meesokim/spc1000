/*
	wav2tap tool by F.Frances
	version 1.1: fixed a difference between wav2tap's behavior and
	the real Oric: the Oric accepts non-$16 values after the three
	$16 bytes (four when counting the first synchronized byte) and
	before the starting $24 byte. Thanks to Simon for spotting this.
	
	wav2tap for spc-1000 by meeso.kim
	version 1.2: revised for spc-1000 wav file.
*/

#include <ncursesw/ncurses.h>
#include <stdio.h>
#include <unistd.h>

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
int getbyte(FILE *);
int getshort(FILE *);
int getint(FILE *);
int (*getdata)(FILE *);


int main(int argc,char **argv)
{	
	unsigned start,end,byte,v;
	int min, max, idx0, idx, level, imin, imax, height, width;
	int b0, b1, b2, s0, s1, count, high, low, up, down, x, pos, mxi, mni, mxi0, mni0, gap, m2i;
	float t;
	int s, row, col, ww;
	
	s = strlen(argv[0]);
	while(*(argv[0]+s)!='/'&&*(argv[0]+s)!='\\'&&s>=0)s--;
	if (argc<3) { printf("wap2tap special Simon\nUsage: %s file.wav file.tap\n",argv[0]+s+1); exit(1);}
	in=fopen(argv[1],"rb");
	if (in==NULL) { printf("Unable to open WAV file\n"); exit(1);}
	fread(&sample_riff,sizeof(sample_riff),1,in);
//	if (sample_riff.channels!=1 || sample_riff.freq!=4800 || sample_riff.byte_per_sample!=1) {
//		printf("Invalid WAV format: should be 4800 Hz, 8-bit, mono\n");
//		exit(1);
//	}
	//initscr();
	col = 80;//getmaxx(stdscr); 
	mni = mni0 = 0;
	//endwin();
	printf("Channels:%d, Freq=%d, Sampling=%d\n", sample_riff.channels, sample_riff.freq, sample_riff.byte_per_sample);	
	//exit(0);
	switch(sample_riff.byte_per_sample)
	{
		case 1:
			getdata = getbyte;
			level = 0x80;
			height = 1 << (8 - 1);
			break;
		case 2:
			getdata = getshort;
			level = 0;
			height = 1 << (16 - 1);
			break;
		case 4:
			getdata = getint;
			level = 0;
			height = 1 << (32 - 1);
			break;
		default:
			break;
	}
	out=fopen(argv[2],"wb");
	if (out==NULL) { printf("Unable to create TAP file\n"); exit(1);}
	x = 0;
	if (argc > 3)
	{
		pos = atoi(argv[3]);
	}
	else
		pos = - 100;
	max = min = level;
	idx = imin = imax = 0;
	s0 = b0;
	high = low = count = 0;
	up = down = 0;
	b0 = getdata(in);
	if (b0 < 0)
		down = 1;
	else
		up = 1;
	for (;;) {
		b1 = getdata(in);
		//printf("%d\n", b1);
		if (0)
		{
			ww = col/2+(float)b1/(1<<(sample_riff.byte_per_sample*8-1))*col/2; 
			if (ww > 80) ww = 80;
			printf("%d", idx);
			for(s = 0; s < ww; s++)
				putc(' ', stdout);
			printf("*");
		}
		if (b0 <= 0 && b1 > 0)
		{
			if (min < 0 - height * 0.05)
			{
				up = 1;
				down = 0;
				high = 0;
				mxi0 = mxi;
				mxi = idx;
				gap = mni - mni0;
				t = (float)(ftell(in)-sizeof(sample_riff))/sample_riff.freq/sample_riff.byte_per_sample;
				if (max - min > height/3)
				{
					if (gap >= 18 & gap < 30)
					{
						v = 0;
						fputc('0', out);
						x ++;
					} else if (gap > 30)
					{
						v = 1;
						fputc('1', out);
						x ++;
					}
					else
					{
						v = -1;
					}
					printf("\n%d @%d:%02.5f(%ld), h=%d, min=%d, max=%d, mingap=%d(%d,%d), min2this=%d", v, ((int)t)/60, t-((int)t)/60*60, x, high, min, max, gap, mni, mni0, m2i);
				}
				max = b1;
			}
			else
			{
				v = -2;
				printf("\n%d @%d:%02.5f(%ld), h=%d, min=%d, max=%d, mingap=%d(%d,%d), min2this=%d", v, ((int)t)/60, t-((int)t)/60*60, x, high, min, max, gap, mni, mni0, m2i);
			}
		}
		else if (b0 > 0 && b1 <= 0)
		{
			if (min < 0 - height * 0.05)
			{
				down = 1;
				up = 0;
				low = 0;
				gap = mni - mni0;
				m2i = idx - idx0;
				/*
				if (max - min > height/3)
				{
					t = (float)(ftell(in)-sizeof(sample_riff))/sample_riff.freq/sample_riff.byte_per_sample;
					if (m2i > 27 || (high >= 14 && (max - min) > height/2) || high > 15)
					{
						v = 1;
						fprintf(out, "1");
						printf("%d @%d:%02.5f(%ld), h=%d, min=%d, max=%d, mingap=%d, min2this=%d\n", v, ((int)t)/60, t-((int)t)/60*60, x, high, min, max, gap, m2i);
						x ++;
					}
					else if (m2i > 20 || high <= 20 && high >= 5)
					{
						v = 0;
						fprintf(out, "0");
						printf("%d @%d:%02.5f(%ld), h=%d, min=%d, max=%d, mingap=%d, min2this=%d\n", v, ((int)t)/60, t-((int)t)/60*60, x, high, min, max, gap, m2i);
						x ++;
					}
					else 
					{
						v = -1;
						//printf("max=%d, min=%d, height=%d, width=%d, high=%d, min2this=%d\n", max, min, max - min, gap, high, m2i);
	//					printf("%d fpos time=%d:%02.5f, h=%d, min=%d, max=%d\n", v, ((int)t)/60, t-((int)t)/60*60, high, min, max);
	//					exit(0);
					}
					if (x > (pos - 32) && x <= pos + 1)
					{
						//printf("max=%d, min=%d, height=%d, error=%d\n", max, min, max - min, high);
						printf("%d @%d:%02.5f, h=%d, min=%d, max=%d, mingap=%d, min2this=%d\n", v, ((int)t)/60, t-((int)t)/60*60, high, min, max, gap, m2i);
					}
				}
				*/
				min = b0;
				if (gap > 10 && gap < 30)
				{
					//fprintf(out, "0");
				} else if (gap > 33)
				{
					//fprintf(out, "1");
				} else
				{
					//printf("min to min:%d\n", mni - mni0);
				}
				mni0 = mni;
				mni = idx;
				idx0 = idx;
			}
			else
			{
				v= -3;
				printf("\n%d @%d:%02.5f(%ld), h=%d, min=%d, max=%d, mingap=%d(%d,%d), min2this=%d", v, ((int)t)/60, t-((int)t)/60*60, x, high, min, max, gap, mni, mni0, m2i);
			}
		}
		max = (max < b1 ? b1 : max);
		mxi = (max < b1 ? idx : mxi);
		if (min > b1)
		{
			min = b1;
			mni = idx;
			//printf("MIN");
		}
		high += up;
		low += down;
		idx++;
		b0 = b1;
		//printf("\n");
		if (feof(in))
			break;			
	}
	printf("fpos=%f\n", (float)(ftell(in)-sizeof(sample_riff))/sample_riff.freq/sample_riff.byte_per_sample);
	
	return 0;
}

int getbyte(FILE *f)
{
	unsigned char a;
	a = getc(f) ;
	return ((int)a) - 0x80;
}
int getshort(FILE *f)
{
	int a, b, c;
	b = getc(f);
	c = getc(f);
	a = b + c * 256;
//	printf("%d ", a);
	a = (short) a;
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

int getbit()
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