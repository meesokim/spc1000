#include "cassette.h"

/**
 * Reset Cassette structure
 * @param cas cassette structure
 */
void ResetCassette(CassetteTape *cas)
{
	cas->rdVal = -2;
	cas->cnt0 = cas->cnt1 = 0;

	cas->wrVal = 0; // correct location?
	cas->wrRisingT = 0;
}

/**
 * Initialize cassette structure
 * @param cas cassette structure
 */
void InitCassette(CassetteTape *cas)
{
	cas->button = CAS_STOP;

	spconf.wfp = NULL;
	spconf.rfp = NULL;
	ResetCassette(cas);
}

int ReadVal(void)
{
	int c;
	static int b = 0;
	if (spconf.rfp != NULL)
	{
		static int EOF_flag = 0;
		if (b > 50) 
		{
			//printf("\b b=%d\n", b);
			while(fgetc(spconf.rfp) == '0');
			c = '1';
			b = 0;
		}
		else
			c = fgetc(spconf.rfp);
		if (c == EOF)
		{
			if (!EOF_flag)
				printf("EOF\n"), EOF_flag = 1;
			c = -1;
		}
		else
		{
			EOF_flag = 0;
			c -= '0';
		}
		b = (c == 0 ? b+1 : 0);
		return c;
	}
	return -1;
}

#define STONE 2
#define LTONE (STONE*2)
int CasRead(CassetteTape *cas)
{
	int curTime;
	int bitTime;
	int ret = 0;
	int t;

	t = (spcsys.cycles - cas->lastTime) >> 5;
	printf("%d\n", t);
	if (t > (cas->rdVal ? LTONE : STONE))
	{
		cas->rdVal = ReadVal();
		printf("%d %d\n",spcsys.cycles-cas->lastTime, cas->rdVal);
		cas->lastTime = spcsys.cycles;
		t = (spcsys.cycles - cas->lastTime) >> 5;		
	}
	switch (cas->rdVal)
	{
	case 0:
		if (t > STONE/2)
			ret = 1; // high
		else
			ret = 0; // low
        break;
	case 1:
		if (t > STONE)
			ret = 1; // high
		else
			ret = 0; // low
	}
	return ret; // low for other cases
}

void CasWrite1(CassetteTape *cas, int val)
{
	fputc('0'+val, spconf.wfp);
	printf("%d", val);
	cas->wrVal = val;
	return;
}

void CasWrite(CassetteTape *cas, int val)
{
	UINT32 curTime;
	int t;

	t = (spcsys.cycles - cas->lastTime) >> 5;
	if (t > 100)
		cas->cnt0 = cas->cnt1 = 0;
	cas->lastTime = spcsys.cycles;
	if (cas->wrVal == 1)
	{
		if (val == 0)
			if (t > STONE/2) 
			{
				printf("1");
				cas->cnt0 = 0;
				fputc('1', spconf.wfp);
			} else {
				if (cas->cnt0++ < 100)
				{
					printf("0");
					fputc('0', spconf.wfp);
				}
			}
	}
	cas->wrVal = val;
}
