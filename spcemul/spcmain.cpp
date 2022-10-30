/**
 * @file spcmain.cpp
 * @brief SPC-1000 emulator main (in,out, and main loop)
 * @author Kue-Hwan Sihn (ionique)
 * @date 2005~2006.10
 */

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include "Z80.h"    // Z-80 emulator (Marat Fayzullin)
#include "MC6847.h" // Video Display Chip (ionique)
#include "AY8910.h" // AY-3-8910 Sound (Marat Fayzullin)
#include "common.h"
#include "gifsave.h"
#include "tms9918.h"

int SaveImageFile(char *);
int LoadImageFile(char *);
extern void save_s1sfile();
extern void load_s1sfile();
extern void floppy_disk_menu();
extern void initDebug();
extern Uint32 colorMap[];
extern Uint16 cMap[];
PIXEL *fb;
#ifdef __cplusplus
}
#endif
tms9918 t99;
#include "spckey.h" // keyboard definition

#define I_PERIOD 4000
#define TURBO (spconf.turbo) 
#define I_PERIOD_TURBO (I_PERIOD * (TURBO + 1))
#define INTR_PERIOD 16.6666

#define SPC_EMUL_VERSION "1.2 (2014.04.22)"
#define FCLOSE(x)	fclose(x),(x)=NULL

enum casmode {CAS_STOP, CAS_PLAY, CAS_REC};

TTF_Font *font = NULL;
//#define DEBUG_MODE
int OpenTapeFile(void);
int SaveAsTapeFile(void);
void ProcessKeyDown(SDLKey sym);
void ProcessKeyUp(SDLKey sym);

char load_path_taps[MAX_PATH_LENGTH];
char path_taps[MAX_PATH_LENGTH];
char path_disks[MAX_PATH_LENGTH];

SPCConfig spconf;
SPCSystem spcsys;
SPCSimul  spcsim;
SDLInfo   spcsdl;
int t;

extern byte *z80mem;

void ToggleFullScreen();
void SDL_Resize(SDL_ResizeEvent &resize);

void loadROM(const char *name)
{
	FILE *fp;
	if ((fp = fopen(name, "rb")) == NULL)
	{
		printf("spcall.rom (32KB) not found.\n");
		exit(1);
	}
	fread(spcsys.ROM, 1, 32768, fp);	// read ROM file
	fclose(fp);
}
/*************************************************************/
/** INI file processing                                     **/
/*************************************************************/

/**
 * Strip white space in the given string
 * @param x string pointer to strip (in-place update)
 */
void StripWS(char *x)
{
	char *tmp = x;

	for (; *x != '\0'; x++)
		if (strchr("\t\r\n", *x) == NULL)
			*tmp = *x, tmp++;
	*tmp = '\0';
}

/**
 * Compare string, only to the length of first string
 * @param keyword the first string to compare
 * @param str the second string
 */
int str1ncmp(const char *keyword, const char *str)
{
	return strncmp(keyword, str, strlen(keyword));
}

/**
 * Get a integer value from the string of "KEYWORD=VALUE" form
 * @param str the string of "KEYWORD=VALUE" form
 */
int GetVal(char *str)
{
	char *tmp;

	tmp = strchr(str, '=');
	if (tmp == NULL)
		return -1;
	return atoi(tmp+1);
}

char *GetString(char *str)
{
	char *tmp, *x;

	tmp = strchr(str, '=');
	str = tmp + 1;
	x = tmp;
	if (tmp == NULL)
		return (char*)NULL;
	for (; *x != '\0'; x++)
		if (strchr("\"", *x) == NULL)
			*tmp = *x, tmp++;
	*tmp = '\0';
	printf("%s\n", str);
	return str;
}

/**
 * Process INI file
 * @param fname INI file name
 */
void ReadINI(char *fname)
{
	FILE *fp;
	static char inputstr[120];
	// default
	spconf.colorMode = SPCCOL_NEW1;
	spconf.scanLine = SCANLINE_045_ONLY;
	spconf.frameRate = 30;
	spconf.casTurbo = 1;
	spconf.snd_vol = 5;

	if ((fp = fopen(fname, "rt")) == NULL)
	{
		printf("INI file \"%s\" not found.\n", fname);
		return;
	}

	while (1)
	{
		int val = 0;

		if (fgets(inputstr, 120, fp) == NULL)
			break;

		StripWS(inputstr);

		if (!strcmp("", inputstr) || inputstr[0] == '#')
			continue;
		if (!str1ncmp((const char *)"COLORSET", inputstr))
		{
			val = GetVal(inputstr);
			if (val >= 0 && val <= SPCCOL_GREEN)
				spconf.colorMode = val;
			continue;
		}
		if (!str1ncmp((const char *)"SCANLINE", inputstr))
		{
			val = GetVal(inputstr);
			if (val >= 0 && val <= SCANLINE_045_ONLY)
				spconf.scanLine = val;
			continue;
		}
		if (!str1ncmp((const char *)"FRAMERATE", inputstr))
		{
			val = GetVal(inputstr);
			if (val >= 0)
				spconf.frameRate = val;
			continue;
		}
		if (!str1ncmp((const char *)"CASTURBO", inputstr))
		{
			val = GetVal(inputstr);
			if (val == 0 || val == 1)
				spconf.casTurbo = val;
			continue;
		}
		if (!str1ncmp((const char *)"FULLSCREEN", inputstr))
		{
			val = GetVal(inputstr);
			if (val != 0)
				spconf.fullscreen = 1; 
			continue;
		}
		if (!str1ncmp((const char *)"SOUNDVOL", inputstr))
		{
			val = GetVal(inputstr);
			if (val >= 0 && val <= 40)
				spconf.snd_vol = val;
			continue;
		}
		if (!str1ncmp("PCKEYBOARD", inputstr))
		{
			val = GetVal(inputstr);
			if (val >= 0)
            {
				spconf.pcKeyboard = val;
            }
			continue;
		}
		if (!str1ncmp("FONTSIZE", inputstr))
		{
			val = GetVal(inputstr);
			if (val >= 0)
            {
				spconf.font_size = val;
            }
			continue;
		}
		if (!str1ncmp("FONT", inputstr))
        {
            char *str;
            str = GetString(inputstr);
            if (str != NULL)
            {
                strcpy(spconf.font_name, str);
            }
            continue;
        }
		if (!str1ncmp("TAPEPATH", inputstr))
        {
            char *str;
            str = GetString(inputstr);
            if (str != NULL)
            {
                strcpy(load_path_taps, str);
            }
            continue;
        }
		if (!str1ncmp("DISKPATH", inputstr))
        {
            char *str;
            str = GetString(inputstr);
            if (str != NULL)
            {
                strcpy(path_disks, str);
                str = &path_disks[strlen(path_disks)-1];
                if (*str != '/') {
                    strcat(path_disks, "/");
                }
            }
            continue;
        }
		if (!str1ncmp("DISK1", inputstr))
        {
            char *str;
            str = GetString(inputstr);
            if (str != NULL)
            {
                if (strstr(str, "/") > 0)
                    strcpy(spcsys.fdd.diskfile, str);
                else
                {
                    strcpy(spcsys.fdd.diskfile, path_disks);
                    strcat(spcsys.fdd.diskfile, str);
                }
                printf("disk1=%s\n", spcsys.fdd.diskfile);
            }
            continue;
        }
        if (!str1ncmp("DISK2", inputstr))
        {
            char *str;
            str = GetString(inputstr);
            if (str != NULL)
            {
                if (strstr(str, "/") > 0)
                    strcpy(spcsys.fdd.diskfile2, str);
                else
                {
                    strcpy(spcsys.fdd.diskfile2, path_disks);
                    strcat(spcsys.fdd.diskfile2, str);
                }
                printf("disk2=%s\n", spcsys.fdd.diskfile2);
            }
            continue;
        }
		printf("The following line in the INI file is ignored:\n\t%s\n", inputstr);
	}
	fclose(fp);
}

/*************************************************************/
/** Initialize IO                                           **/
/*************************************************************/

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

/**
 * Initialize I/O space structure
 * @param void
 */
void InitIOSpace(void)
{
	spcsys.IPLK = 1;
	spcsys.GMODE = 0;
	spcsys.cas.motor = 0;
	spcsys.cas.pulse = 0;
	spcsys.psgRegNum = 0;
	// VRAM init?
	Reset8910(&(spcsys.ay8910), 0);
	TURBO = 0;
	memset(spcsys.keyMatrix, 0xff, 10);
}

/*************************************************************/
/** Memory Read/Write                                       **/
/*************************************************************/

/**
 * Writing SPC-1000 RAM
 * @param Addr 0x0000~0xffff memory address
 * @param Value a byte value to be written
 */
void WrZ80(register word Addr,register byte Value)
{
	spcsys.RAM[Addr] = Value;
//	printf("0x%04x-%02x\n", Addr, Value);
}

/**
 * Reading SPC-1000 RAM
 * @param Addr 0x0000~0xffff memory address
 * @warning Read behavior is ruled by IPLK
 */

#if WIN32
void CheckKeyboard(void);
int OpenTapeFile(void);
int SaveAsTapeFile(void);
void flushSDLKeys(void);
#else
#include <time.h>
long timeGetTime()
{
    struct timespec tspec;
    clock_gettime(CLOCK_MONOTONIC, &tspec);
    return tspec.tv_sec * 1000 + tspec.tv_nsec/1.0e6;
}
#endif
/*************************************************************/
/** Image File (Z80+Memory+IO+VRAM Save File) Read/Write    **/
/*************************************************************/
const char *signiture = "0123456789ENDOFS1S\0";
int LoadImageFile(char *szFile)
{
	FILE *fp;
    if ((fp = fopen(szFile,"rb")) == NULL)
    {
        printf("File(%s) (read) open error.\n", szFile);
        return -1;
    }
    char strHeader[100];
    char strVersion[10];
    char strTail[100];
    int i = 0, ch;
    while((ch = fgetc(fp)) != 10)
    {
        printf("%c", ch);
        strHeader[i++]=ch;

    }
    strHeader[i] = 0;
    i = 0;
    fgets(strVersion, 4, fp);
    printf("\n");
    fpos_t fpos;
    fgetpos(fp, &fpos);
    fgetc(fp);
    printf("file=%d", fpos);
    //fscanf(fp, "%s\n%s", strHeader, strVersion);
    printf("Header=%s\nVersion=%s\n", strHeader, strVersion);
    if (!strcmp(strVersion, "1.0"))
    {
        fread(&spcsys.RAM,  sizeof(spcsys.RAM),  1, fp);
        printf("RAM-%x\n", spcsys.RAM[0]);
        fread(&spcsys.VRAM, sizeof(spcsys.VRAM), 1, fp);
        fread(&spcsys.GMODE,sizeof(spcsys.GMODE),1, fp);
        fread(&spcsys.IPLK, sizeof(spcsys.IPLK), 1, fp);
        fread(&spcsys.Z80R, sizeof(spcsys.Z80R), 1, fp);
        fread(&spcsys.tick, sizeof(spcsys.tick), 1, fp);
        fread(&spcsim,      sizeof(spcsim),      1, fp);
        fread(&strTail,     sizeof(strlen(signiture)),    1, fp);
    }
    printf("Tail-%s\n", strTail);
    fclose(fp);
    return 0;
}

int SaveImageFile(char *szFile)
{
	FILE *fp;
    if ((fp = fopen(szFile,"wb")) == NULL)
    {
        printf("File(%s) (write) open error.\n", szFile);
        return -1;
    }
    fputs("S1S SPC-1000 Snapshot Format\n1.0", fp);
    fputc(0, fp);
    fwrite(&spcsys.RAM,  sizeof(spcsys.RAM),  1, fp);
    fwrite(&spcsys.VRAM, sizeof(spcsys.VRAM), 1, fp);
    fwrite(&spcsys.GMODE,sizeof(spcsys.GMODE),1, fp);
    fwrite(&spcsys.IPLK, sizeof(spcsys.IPLK), 1, fp);
    fwrite(&spcsys.Z80R, sizeof(spcsys.Z80R), 1, fp);
    fwrite(&spcsys.tick, sizeof(spcsys.tick), 1, fp);
    fwrite(&spcsim,      sizeof(spcsim),      1, fp);
    fprintf(fp, "0123456789ENDOFS1S\n");
    fclose(fp);
    return 0;
//	printf("File save canceled.\n");
//	return -1;
}
/*************************************************************/
/** Cassette Tape Processing                                **/
/*************************************************************/

int extcmp(char str[], char *cmp)
{
    int len = strlen(str);
    int e = strlen(cmp);
    int s = len - e;
    int i = 0;
    if (len < e)
        return -1;
    for (i = 0; i < e; i++)
    {
        if (tolower(str[s+i]) != tolower(cmp[i]))
            return -1;
    }
    return 0;
}

int OpenTapeFile(void)
{
    int t = 0;
    if (strlen((const char*)spconf.current_tap)>0)
    {

		if ((spconf.rfp = fopen((const char *)spconf.current_tap ,"rb")) == NULL)
		{
			printf("File(%s) (read) open error.\n", spconf.current_tap);
			return -1;
		}
		while(fgetc(spconf.rfp)=='0');
		t = ftell(spconf.rfp);
		fseek(spconf.rfp, (t > 10 ? t - 10 : t) , 0);
		printf("File(%s) (read) opened. %d\n", spconf.current_tap, ftell(spconf.rfp));
		return 0;
    }
    else
        taps_menu();
}

int SaveAsTapeFile(void)
{
	char szFile[256];
#if 0
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name
	HWND hwnd = NULL;       // owner window
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;

	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = 260;
	ofn.lpstrFilter = "Tape\0*.TAP\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	strncpy(szFile, (const char*) &(spcsys.RAM[0x1397]),16);
	szFile[16] = '\0';
	strcat(szFile, ".TAP");

	// Display the Open dialog box.
	if (GetSaveFileName(&ofn)==TRUE)
#endif
	{
		strcpy(szFile, (const char*)spconf.current_tap);
#ifdef DEBUG_MODE
		printf("Writing %s\n", szFile);
#endif
		if (spconf.wfp != NULL)
			fclose(spconf.wfp);
		if ((spconf.wfp = fopen((const char*)szFile,"wb")) == NULL)
		{
			printf("File(%s) (write) open error.\n", szFile);
			return -1;
		}
		printf("File(%s) (write) created.\n", szFile);
		return 0;
	}
	printf("File save canceled.\n");
	return -1;
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
	Uint32 curTime;
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

/*************************************************************/
/** Output I/O Processing                                   **/
/*************************************************************/

/**
 * Put a value on Z-80 out port
 * @param Port 0x0000~0xffff port address
 * @param Value a byte value to be written
 * @warning modified to accomodate 64K I/O address from original Marat's source
 */
 int fcno = 0;
 char dfile[256];
void OutZ80(register word Port,register byte Value)
{
    //printf("0h%04x < 0h%02x\n", Port, Value);

	if (Port < 0x2000) // VRAM area
	{
		spcsys.VRAM[Port] = Value;
	}
	else if ((Port & 0xE000) == 0xA000) // IPLK area
	{
		spcsys.IPLK = (spcsys.IPLK)? 0 : 1;	// flip IPLK switch
		if (spcsys.IPLK)
            z80mem = spcsys.ROM;
        else
            z80mem = spcsys.RAM;
	}
	else if ((Port & 0xE000) == 0x2000)	// GMODE setting
	{
	    //printf("GMODE=%d\n", Value);
		spcsys.GMODE = Value;
#ifdef DEBUG_MODE
		printf("GMode:%02X\n", Value);
#endif
	}
	else if ((Port & 0xE000) == 0x6000) // SMODE
	{
		if (spcsys.cas.button != CAS_STOP)
		{

			if ((Value & 0x02)) // Motor
			{
				if (spcsys.cas.pulse == 0)
				{
					spcsys.cas.pulse = 1;

				}
			}
			else
			{
				if (spcsys.cas.pulse)
				{
					spcsys.cas.pulse = 0;
					if (spcsys.cas.motor)
					{
						spcsys.cas.motor = 0;
#ifdef DEBUG_MODE
						printf("Motor Off\n");
#endif
					}
					else
					{
						spcsys.cas.motor = 1;
#ifdef DEBUG_MODE
						printf("Motor On\n");
#endif
						spcsys.cas.lastTime = 0;
						ResetCassette(&spcsys.cas);
					}
				}
			}
		}

		if (spcsys.cas.button == CAS_REC && spcsys.cas.motor)
		{
			CasWrite1(&spcsys.cas, Value & 0x01);
		}
	}
	else if ((Port & 0xFFFE) == 0x4000) // PSG
	{

		if (Port & 0x01) // Data
		{
		    if (spcsys.psgRegNum == 15) // Line Printer
            {
                if (Value != 0)
                {
                    spcsys.prt.bufs[spcsys.prt.length++] = Value;
//                    printf("PRT <- %c (%d)\n", Value, Value);
//                    printf("%s(%d)\n", spcsys.prt.bufs, spcsys.prt.length);
                }
            }
			//printf("reg:%d,%d\n", spcsys.psgRegNum, Value);
			Write8910(&spcsys.ay8910, (byte) spcsys.psgRegNum, Value);
		}
		else // Reg Num
		{
			spcsys.psgRegNum = Value;
			WrCtrl8910(&spcsys.ay8910, Value);
		}
	}
	else if ((Port & 0x4003) == 0x4003)
	{
		if (fcno < 256)
		{
			dfile[fcno++] = (Value == 0xff ? 0 : Value);
		}
		if (Value == 0)
		{
			fcno = 0;
			FCLOSE(spconf.rfp);
			sprintf((char *)spconf.current_tap, "..\\tape\\%s.tap", dfile);
			printf("%s\n", (char *)spconf.current_tap);
            if (OpenTapeFile() < 0)
                return;
            spcsys.cas.button = CAS_PLAY;
            spcsys.cas.motor = 1;
            spcsys.cas.lastTime = 0;
            ResetCassette(&spcsys.cas);
            printf("file found\n");
		}
		else if (Value == 0xff)
		{
			fcno = 0;
			FCLOSE(spconf.wfp);
			sprintf((char *)spconf.current_tap, "..\\tape\\%s.tap", dfile);
            if (SaveAsTapeFile() < 0)
                return;
            spcsys.cas.button = CAS_REC;
            spcsys.cas.motor = 1;
            spcsys.cas.lastTime = 0;
            ResetCassette(&spcsys.cas);
            printf("file created\n");
		}
	}
	else if (Port & 0x4004)
	{
		if (spconf.wfp)
		{
			printf("%02x", Value);
			fputc('1', spconf.wfp);
			for(int i = 0; i < 8; i++)
				fputc((Value >> (7 - i) & 1) + '0', spconf.wfp);
		}
	}
	else if (Port & 0x4005)
	{
		if (spconf.wfp)
		{
			FCLOSE(spconf.wfp);
		}
		printf("spconf.wfp closed\n");
	}
	else if ((Port & 0xFFF0) == 0xC000)
    {
        if (Port == 0xc000) // PortA
        {
            spcsys.fdd.wrVal = Value;
            //printf("FDD_Data < 0h%02x\n", Value);
        }
        else if (Port == 0xc002) // PortC
        {
            //printf("FDD_PC < 0h%02x\n", Value);
            if (Value != 0)
            {
                spcsys.fdd.PC.b = (Value & 0xf0) | (spcsys.fdd.PC.b & 0xf);
            }
            else
            {
                spcsys.fdd.PC.b = spcsys.fdd.PC.b & 0xf;
            }
            if (Value & 0x10) // DAV=1
            {
                //printf("FDD Data Valid (c000 bit4 on)\n");
                if (spcsys.fdd.isCmd == 1) // command
                {
                    spcsys.fdd.isCmd = 0;
                    printf("FDD Command(Data) = 0h%02x\n", spcsys.fdd.wrVal);
                    spcsys.fdd.cmd = spcsys.fdd.wrVal;
                    switch (spcsys.fdd.wrVal)
                    {
                        case 0x00: // FDD Initialization
                            printf("*FDD Initialization\n");
                            break;
                        case 0x01: // FDD Write
                            printf("*FDD Write\n");
                            spcsys.fdd.rSize = 4;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x02: // FDD Read
                            printf("*FDD Read\n");
//                            spcsys.fdd.PC.bits.rRFD = 1;
                            spcsys.fdd.rSize = 4;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x03: // FDD Send Data
                            //printf("*FDD Send Data\n");
                            if (spcsys.fdd.seq == 4)
                            {
                                printf("seq=%d,(%d,%d,%d,%d)\n", spcsys.fdd.seq, spcsys.fdd.data[0], spcsys.fdd.data[1], spcsys.fdd.data[2], spcsys.fdd.data[3]);
                                spcsys.fdd.buffer = (byte*)(spcsys.fdd.data[1] != 0 ? spcsys.fdd.diskdata2 : spcsys.fdd.diskdata);
                                spcsys.fdd.buffer += ((int)spcsys.fdd.data[2] * 16 + (int)spcsys.fdd.data[3]-1) * 256;
                                spcsys.fdd.datasize = spcsys.fdd.data[0] * 256;
                                spcsys.fdd.dataidx = 0;

                            }
                            spcsys.fdd.rdVal = 0;
                            break;
                        case 0x04: // FDD Copy
                            printf("*FDD Copy\n");
                            spcsys.fdd.rSize = 7;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x05: // FDD Format
                            printf("*FDD Format\n");
                            break;
                        case 0x06: // FDD Send Status
                            printf("*FDD Send Status\n");
    #define DATA_OK 0x40
                            spcsys.fdd.rdVal = 0x80 & DATA_OK;
                            break;
                        case 0x07: // FDD Send Drive State
                            printf("*FDD Send Drive State\n");
    #define DRIVE0 0x10
                            spcsys.fdd.rdVal = 0x0f | DRIVE0;
                            break;
                        case 0x08: // FDD RAM Test
                            printf("*FDD RAM Test\n");
                            spcsys.fdd.rSize = 4;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x09: // FDD Transmit 2
                            printf("*FDD Transmit 2\n");
                            spcsys.fdd.rSize = 4;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x0A: // FDD Action
                            printf("*FDD No Action\n");
                            break;
                        case 0x0B: // FDD Transmit 1
                            printf("*FDD Transmit 1\n");
                            spcsys.fdd.rSize = 4;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x0C: // FDD Receive
                            printf("*FDD Receive\n");
                            spcsys.fdd.rSize = 4;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x0D: // FDD Go
                            printf("*FDD Go\n");
                            spcsys.fdd.rSize = 2;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x0E: // FDD Load
                            printf("*FDD Load\n");
                            spcsys.fdd.rSize = 6;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x0F: // FDD Save
                            printf("FDD Save\n");
                            spcsys.fdd.rSize = 6;
                            spcsys.fdd.seq = 0;
                            break;
                        case 0x10: // FDD Load and Go
                            printf("*FDD Load and Go\n");
                            break;

                    }
                }
                else
                {
                    if (spcsys.fdd.rSize-- > 0)
                    {
                        printf("seq=%d, data = 0x%02x\n", spcsys.fdd.seq, spcsys.fdd.wrVal);
                        spcsys.fdd.data[spcsys.fdd.seq++] = spcsys.fdd.wrVal;
                        // printf("cmd=%d\n", spcsys.fdd.cmd);
                        if (spcsys.fdd.rSize == 0)
                        {
                            int offset = ((int)spcsys.fdd.data[2] * 16 + (int)spcsys.fdd.data[3]-1) * 256;
                            //printf("Fdd Command(%d) Fired\n", spcsys.fdd.cmd);

                            if (spcsys.fdd.cmd == 0x0e)
                            {
                                printf("load(%d,%d,%d,%d),offset=%d\n", spcsys.fdd.data[0], spcsys.fdd.data[1], spcsys.fdd.data[2], spcsys.fdd.data[3], offset);
                                spcsys.fdd.buffer = (byte*)(spcsys.fdd.data[1] != 0 ? spcsys.fdd.diskdata2 : spcsys.fdd.diskdata);
                                spcsys.fdd.buffer += offset;
                                spcsys.fdd.datasize = spcsys.fdd.data[0] * 256;
                                int addr = ((unsigned short)spcsys.fdd.data[4]) * 0x100 + (unsigned short)spcsys.fdd.data[5];
                                printf("target addr=%04x, size=%d\n", addr, spcsys.fdd.datasize);
                                memcpy(&spcsys.RAM[addr], spcsys.fdd.buffer, spcsys.fdd.datasize);
                            }
                            else if (spcsys.fdd.cmd == 0x0f)
                            {
                                printf("save(%d,%d,%d,%d),offset=%d\n", spcsys.fdd.data[0], spcsys.fdd.data[1], spcsys.fdd.data[2], spcsys.fdd.data[3], offset);
                                spcsys.fdd.buffer = (byte*)(spcsys.fdd.data[1] != 0 ? spcsys.fdd.diskdata2 : spcsys.fdd.diskdata);
                                spcsys.fdd.buffer += offset;
                                spcsys.fdd.datasize = spcsys.fdd.data[0] * 256;
                                int addr = ((unsigned short)spcsys.fdd.data[4]) * 0x100 + (unsigned short)spcsys.fdd.data[5];
                                printf("source addr=%04x, size=%d\n", addr, spcsys.fdd.datasize);
                                memcpy(spcsys.fdd.buffer, &spcsys.RAM[addr], spcsys.fdd.datasize);
                            }
                        }
                    }
                }
                spcsys.fdd.PC.bits.rDAC = 1;

            }
            else if (spcsys.fdd.PC.bits.rDAC == 1) // DAV=0
            {
                //printf("FDD Ouput Data Cleared (c000 bit4 off)\n");
                spcsys.fdd.wrVal = 0;
//              printf("FDD_Read = 0h%02x (cleared)\n", spcsys.fdd.wrVal);
                spcsys.fdd.PC.bits.rDAC = 0;
            }
            if (Value & 0x20) // RFD=1
            {
//                printf("FDD Ready for Data Read (c000 bit5 on)\n");
                spcsys.fdd.PC.bits.rDAV = 1;
            }
            else if (spcsys.fdd.PC.bits.rDAV == 1) // RFD=0
            {
                //spcsys.fdd.rdVal = 0;
                //printf("FDD Input Data = 0h%02x\n", spcsys.fdd.rdVal);
                spcsys.fdd.PC.bits.rDAV = 0;
            }
            if (Value & 0x40) // DAC=1
            {
//                printf("FDD Data accepted (c000 bit6 on)\n");
            }
            if (Value & 0x80) // ATN=1
            {
//              printf("FDD Attention (c000 bit7 on)\n");
                //printf("Command = 0x%02x\n", spcsys.fdd.rdVal);
                //printf("FDD Ready for Data\n", spcsys.fdd.rdVal);
                spcsys.fdd.PC.bits.rRFD = 1;
                spcsys.fdd.isCmd = 1;
            }
//          else if (spcsys.fdd.PC.bits.rRFD == 1)
//          {
//              printf("FDD Output Data required.\n");
//              //spcsys.fdd.PC.bits.rRFD = 0;
//          }
        }
    }
}

/**
 * Read a value on Z-80 port
 * @param Port 0x0000~0xffff port address
 * @warning modified to accomodate 64K I/O address from original Marat's source
 */

 int FDD_rData;
 int FDD_wData;

byte InZ80(register word Port)
{
	if (Port >= 0x8000 && Port <= 0x8009) // Keyboard Matrix
	{
		// if (!(spcsys.cas.motor && spconf.casTurbo))
			// CheckKeyboard();
		switch (Port)
		{
		case 0x8000:
		case 0x8001:
		case 0x8002:
		case 0x8003:
		case 0x8004:
		case 0x8005:
		case 0x8006:
		case 0x8007:
		case 0x8008:
		case 0x8009:
		    if (Port == 0x8009 && spcsys.IPL_SW == 1)
            {
                spcsys.IPLK = 0;
                spcsys.IPL_SW = 0;
                return spcsys.keyMatrix[Port-0x8000] & 0xfe;
            }
            return spcsys.keyMatrix[Port-0x8000];
			break;
        case 0x9000:
        case 0x9001:
        case 0x9002:
            break;
		}
		return 0xff;
	}
	else if ((Port & 0xFFF0) == 0xc000)
    {
        if (Port == 0xc001)
        {
            if (spcsys.fdd.cmd == 0x3)
            {
                spcsys.fdd.rdVal = *(spcsys.fdd.buffer + spcsys.fdd.dataidx++);
            }
            //printf("FDD_Data > 0h%02x\n", spcsys.fdd.rdVal);
            return spcsys.fdd.rdVal;
        }
        else if (Port & 0xc002)
        {
            //printf("FDD_PC > 0h%02x\n", spcsys.fdd.PC.b);
            return spcsys.fdd.PC.b;
        }

    }
	else if ((Port & 0xE000) == 0xA000) // IPLK
	{
		spcsys.IPLK = (spcsys.IPLK)? 0 : 1;
		if (spcsys.IPLK)
            z80mem = spcsys.ROM;
        else
            z80mem = spcsys.RAM;
	}
	else if ((Port & 0xE000) == 0x2000) // GMODE
	{
		return spcsys.GMODE;
	}
	else if ((Port & 0xE000) == 0x0000) // VRAM reading
	{
		return spcsys.VRAM[Port];
	}
	else if ((Port & 0xFFFE) == 0x4000) // PSG
	{
		byte retval = 0x1f;
		if (Port & 0x01) // Data
		{
			if (spcsys.psgRegNum == 14)
			{
				// 0x80 - cassette data input
				// 0x40 - motor status
				// 0x20 - print status
//				if (spcsys.prt.poweron)
//                {
//                    printf("Print Ready Check.\n");
//                    retval &= 0xcf;
//                }
//                else
//                {
//                    retval |= 0x20;
//                }
				if (spcsys.cas.button == CAS_PLAY && spcsys.cas.motor)
				{
					if (CasRead(&spcsys.cas) == 1)
//					if (ReadVal() == 1)
						retval |= 0x80; // high
					else
						retval &= 0x7f; // low
					//printf("%d", retval & 0x80 ? 1 : 0);
					printf("c:%d\n", spcsys.cycles);
				}
				if (spcsys.cas.motor)
					retval &= (~(0x40)); // 0 indicates Motor On
				else
					retval |= 0x40;

			}
			else 
			{
				int data = RdData8910(&spcsys.ay8910);
				//printf("r(%d,%d)\n", spcsys.psgRegNum, data);
				return data;
			}
		} else if (Port & 0x02)
		{
            retval = (ReadVal() == 1 ? retval | 0x80 : retval & 0x7f);
		}
		return retval;
	}
	else if (Port == 0x4003)
	{
		return (spconf.rfp ? 1 : 0);
	}
	else if (Port == 0x4004)
	{
		byte retval = 0;
		//int pos = ftell(spconf.rfp);
		for(int i = 0; i < 8; i++) 
		{
			if (ReadVal())
				retval += 1 << (7 - i);
		}
		//fseek(spconf.rfp, pos, 0);
		ReadVal();
		//printf("%02x,", retval);
		return retval;
	}
	return 0;
}

void SetPCKeyboard(byte *addr)
{
    unsigned char rawData[26] = {
        0xFE, 0xd0, 0xDA, 0xBA, 0x13, 0xD6, 0x4f, 0x21, 0xC4, 0x13, 0xD6, 0x81,
        0x87, 0x5F, 0x16, 0x00, 0x19, 0xC3, 0xA3, 0x2A, 0x82, 0x33, 0x82, 0x33,
        0x82, 0x33
    };

	printf("PC keyboard mode\n");
	if (spconf.pcKeyboard) // modify ROM data for PC Keyboard
    {
        addr[0x1311] = 0x27; //3b '(AT), :(SPC)
        addr[0x1320] = 0x3d;
        addr[0x1331] = 0x7c;
        addr[0x1341] = 0x5d;
        addr[0x1346] = 0x60;
        addr[0x1350] = 0x7f;
        addr[0x1351] = 0x29;
        addr[0x1352] = 0x3a; //22 :(AT), +(SPC)
        addr[0x1355] = 0x28;
        addr[0x1359] = 0x22; //3a "(AT), *(SPC)
        addr[0x135d] = 0x2a;
        addr[0x1365] = 0x26;
        addr[0x1368] = 0x2b;
        addr[0x136d] = 0x5e; // 53
        addr[0x1379] = 0x5c;
        addr[0x138d] = 0x40;
        addr[0x138e] = 0x7e;
        //memcpy(&addr[0x13b0], rawData, 26);

    }
    else // revert to original ROM data.
    {
        addr[0x1311] = 0x3a;
        addr[0x1320] = 0x40;
        addr[0x1331] = 0x7c;
        addr[0x1341] = 0x5d;
        addr[0x1346] = 0x5e;
        addr[0x1350] = 0x3d;
        addr[0x1351] = 0x30;
        addr[0x1352] = 0x2b;
        addr[0x1355] = 0x29;
        addr[0x1359] = 0x2a;
        addr[0x135d] = 0x28;
        addr[0x1365] = 0x27;
        addr[0x1368] = 0x60;
        addr[0x136d] = 0x26;
        addr[0x1379] = 0x7f;
        addr[0x138d] = 0x22;
        addr[0x138e] = 0x7e;
    }
    spconf.pcKeyboard = !spconf.pcKeyboard;
    return;
}
/*************************************************************/
/** Input I/O Processing                                    **/
/*************************************************************/

/**
 * Keyboard hashing table structure
 */
typedef struct
{
	int numEntry;
	TKeyMap *keys;
} TKeyHashTab;

/**
 * Keyboard Hashing table definition.
 * initially empty.
 */
TKeyHashTab KeyHashTab[256] = { 0, NULL };

/**
 * Build Keyboard Hashing Table
 * Call this once at the initialization phase.
 */
void BuildKeyHashTab(void)
{
	int i;
	static int hashPos[256] = { 0 };

	for (i = 0; spcKeyMap[i].keyMatIdx != -1; i++)
	{
		int index = spcKeyMap[i].sym % 256;

		KeyHashTab[index].numEntry++;
		hashPos[index]++;
	}

	for (i = 0; i < 256; i++)
	{
		KeyHashTab[i].keys
			= (TKeyMap *) malloc(sizeof(TKeyMap) * hashPos[i]);
	}

	for (i = 0; spcKeyMap[i].keyMatIdx != -1; i++)
	{
		int index = spcKeyMap[i].sym % 256;

		hashPos[index]--;
		if (hashPos[index] < 0)
			printf("Fatal: out of range in %s:BuildKeyHashTab().\n",
			__FILE__), exit(1);
		KeyHashTab[index].keys[hashPos[index]]
			= spcKeyMap[i];
	}
}
#ifdef WIN32
    #define CLIP_BOARD_FILE "/dev/clipboard"
#elif defined(LINUX)
    #define CLIP_BOARD_FILE "/dev/clip"
#endif // WIN32

char *GetClipboardText(int size)
{
#ifdef WIN32
    HWND hwnd=0;
    HANDLE htext;
    char *p, *q;

    if (OpenClipboard(hwnd)==0) return NULL;
    htext= GetClipboardData(CF_TEXT);

    if (htext==0) return NULL;
    size=GlobalSize(htext);

    q=(char *)malloc(size);
    if (q==NULL) return NULL;

    p=(char *)GlobalLock(htext);
    memcpy(q,p,size);
    GlobalUnlock(htext);
    CloseClipboard();
    return q;
#elif defined(LINUX)
    FILE *file = fopen(CLIP_BOARD_FILE, "r");
    char *str = (char *)malloc(size);
    int i = 0;
    char ch = 0;
    if (file != NULL)
    {
        while((ch = fgetc(file)) >= 0)
        {
            str[i++] = ch;
        }
    }
    str[i] = 0;
    return str;
#endif // WIN32
}

int SetClipboardText(const char *str)
{
#if WIN32
    int retVal;
    HANDLE hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, strlen(str) + 1);//get handle to memory to hold phrase
    char *ptrData = (char*)GlobalLock(hData);//get pointer from handle
    strcpy(ptrData, str);
    GlobalUnlock(hData);//free the handle
    OpenClipboard(NULL);//allow you to work with clipboard
    EmptyClipboard();//clear previous contents
    SetClipboardData(CF_TEXT, hData);
    CloseClipboard();
    return retVal;
#endif // WIN32
}
static SDL_TimerID m_uiPasteTimer;
static char *m_uiStr;
static int m_uiStrIdx;

void InitUIPaste(void)
{
    m_uiStr = NULL;
    m_uiStrIdx = 0;
}

char ProcessSDLKey(char c)
{
    const char *uppercase = "~!@#$%^&*()_+{}|:\"<>?ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char *lowercase = "`1234567890-=[]\\;',./abcdefghijklmnopqrstuvwxyz";
    unsigned int i;
    for (i = 0; uppercase[i]; i++)
        if (c == uppercase[i])
        {
            ProcessKeyDown(SDLK_LSHIFT);
            c = lowercase[i];
            break;
        }
    if (i == strlen(uppercase))
        ProcessKeyUp(SDLK_LSHIFT);
    ProcessKeyDown((SDLKey)c);
    return c;
}

UINT32 UI_PasteChar(void)
{
    static SDLKey sym = (SDLKey) 0;
    static int keydown = 0;
    static int keyup = 0;
    if (sym == 0)
    {
        if (m_uiStr)
        {
            sym = (SDLKey) m_uiStr[m_uiStrIdx++];
            if (sym == '\n')
                sym = (SDLKey) m_uiStr[m_uiStrIdx++];
        }
        if (sym == 0)
        {
            SDL_RemoveTimer(m_uiPasteTimer);
            ProcessKeyUp(SDLK_LSHIFT);
            free(m_uiStr);
            m_uiStr = NULL;
            m_uiStrIdx =0;
            return 0;
        }
        sym = (SDLKey) ProcessSDLKey(sym);
        return 10;
    }
    else
    {
        ProcessKeyUp(sym);
        if (sym == '\r')
        {
            sym = (SDLKey) 0;
            return 500;
        }
        sym = (SDLKey) 0;
        return 50;
    }
}

void UI_Paste(char *str)
{
    if (strlen(str) > 0)
    {
        m_uiStr = str;
        m_uiStrIdx = 0;
        m_uiPasteTimer = SDL_AddTimer(1, (SDL_NewTimerCallback) UI_PasteChar, NULL);
    }
}
void PRT_Save(const char *p, int size)
{
	FILE *prn = 0;
	int sno = 0;
	char filename[256];
	while(1) {
		sprintf(filename, "print%03d.prn", sno++);
		printf("%d, %s\n", sno, filename);
		if (prn = fopen(filename, "r"))
		{
			fclose(prn);
			continue;
		}
		else
			break;
		
	};
	printf("!%d, %s\n", prn, filename);
	prn = fopen(filename, "wb");
	fwrite(p, 1, size, prn);
	fclose(prn);
	return;
}

extern Uint16 border;
extern unsigned short *frameBuf;
int gpixel(int x, int y)
{
    return *(Uint8 *)(fb + (y * 320 + x));
}
void VDP_Save()
{
	int sno = 0;
	int q=0;
	FILE *snapshot = 0;
	char filename[256];
	while(1) {
		sprintf(filename, "snapshot%03d.gif", sno++);
		printf("%d, %s\n", snapshot, filename);
		if (snapshot = fopen(filename, "r"))
		{
			fclose(snapshot);
			continue;
		}
		else
			break;
		
	};
	printf("!%d, %s\n", snapshot, filename);
	GIF_Create(filename, 320, 240, 12, 2);
	GIF_SetColor(q++, 0x00, 0x00, 0x00);
	GIF_SetColor(q++, 0x07, 0xff, 0x00);
	GIF_SetColor(q++, 0xff, 0xff, 0x00);
	GIF_SetColor(q++, 0x3b, 0x08, 0xff);
	GIF_SetColor(q++, 0xcc, 0x00, 0x3b);
	GIF_SetColor(q++, 0xff, 0xff, 0xff);
	GIF_SetColor(q++, 0x07, 0xe3, 0x99);
	GIF_SetColor(q++, 0xff, 0x1c, 0xff);
	GIF_SetColor(q++, 0xff, 0x81, 0x00);
	
	GIF_SetColor(q++, 0x07, 0xff, 0x00);
	GIF_SetColor(q++, 0xff, 0xff, 0xff);
	
	GIF_SetColor(q++, 0x00, 0x22, 0x00);
	GIF_SetColor(q++, 0x07, 0xff, 0x00);
	GIF_SetColor(q++, 0x91, 0x00, 0x00);
	GIF_SetColor(q++, 0xff, 0x81, 0x00);
	GIF_CompressImage(0,0,-1,-1,gpixel);
	GIF_Close();
	snapshot = fopen(filename, "ab+");
	if (snapshot)
	{
		fprintf(snapshot, "%s", "VDPDUMP.");
		fprintf(snapshot, "%c", (char) spcsys.GMODE);
		for(int i = 0; i < 0x2000; i++)
		{
			fprintf(snapshot, "%c", (char) spcsys.VRAM[i]);
		}
	}
	fclose(snapshot);
}
/**
 * SDL Key-Down processing. Special Keys only for Emulator
 * @param sym SDL key symbol
 */
void ProcessSpecialKey(SDL_keysym ksym)
{
	int index = ksym.sym % 256;
	int retVal;
	FILE *rfp_save;
	FILE *wfp_save;
	char *str;
    int r = 0;
	switch (ksym.sym)
	{
	case SDLK_SCROLLOCK: // turbo mode
		TURBO = (TURBO ? 0: 10);  // toggle
		if (!TURBO) t = timeGetTime();
		printf("turbo %s\n", (TURBO)? "on":"off");
		break;
    case SDLK_INSERT:
        if (m_uiStr == NULL)
        {
            str = GetClipboardText(64000);
            if (str)
            {
                printf("clipboard:%s\n",str);
                UI_Paste(str);
            }
        }
        break;
	case SDLK_PRINT:
    case SDLK_SYSREQ:
        retVal = SetClipboardText((const char *)spcsys.prt.bufs);
		PRT_Save((const char *)spcsys.prt.bufs, spcsys.prt.length);
        printf("Printer Output.(%d)\n%s\n", retVal, spcsys.prt.bufs);
        break;
    case SDLK_F6:
	    if (ksym.mod & KMOD_ALT)
        {
            snapshots_menu();
        } else {
            help_menu();
        }
        break;
    case SDLK_F7:
	    if (ksym.mod & KMOD_ALT)
        {
            settings_menu();
        } else {
            taps_menu();
        }
        break;
	case SDLK_F8: // PLAY button
	    printf("SDLK_F8 pressed\n");
		if (spconf.rfp != NULL)
			FCLOSE(spconf.rfp);
		if (ksym.mod & KMOD_ALT) // STOP button
        {
            spcsys.cas.button = CAS_STOP;
            spcsys.cas.motor = 0;
            printf("stop button\n");
        }
        else // PLAY button
        {
			if (spconf.wfp != NULL)
			{
				spcsys.cas.button = CAS_REC;
				spcsys.cas.motor = 1;
				printf("rec button pushed\n");
 			}
			else if (OpenTapeFile() < 0)
                break;
			else {
				spcsys.cas.button = CAS_PLAY;
				spcsys.cas.motor = 1;
			}
            spcsys.cas.lastTime = 0;
            ResetCassette(&spcsys.cas);
            printf("play button pushed\n");
        }
		break;
	case SDLK_F9: // FDD management
		if (ksym.mod & KMOD_ALT)
		{
			VDP_Save();
		}
		else
		{
			printf("Floppy Disk management\n");
			floppy_disk_menu();
		}
		break;
	case SDLK_F10: // Quit
        SDL_Quit();
        exit(0);
        break;
	case SDLK_PAGEUP: // Image Save        puts("q          : Exit Z80 emulation");
		save_s1sfile();
		printf("Image Save\n");
		break;

	case SDLK_PAGEDOWN: // Image Load
		load_s1sfile();
//		r = 1;
//		spcsys.tick = SDL_GetTicks();
//		spcsim.baseTick = SDL_GetTicks();
//		spcsim.prevTick = spcsim.baseTick;
//		spcsys.cas.button = CAS_STOP;
//		spcsys.cas.motor = 0;
//		if (spcsys.GMODE & 0x08)
//		{
//			SetMC6847Mode(SET_GRAPHIC, spcsys.GMODE);
////			UpdateMC6847Gr(MC6847_UPDATEALL);
//		}
//		else
//		{
//			SetMC6847Mode(SET_TEXTMODE, spcsys.GMODE);
////			UpdateMC6847Text(MC6847_UPDATEALL);
//		}
//		printf("Image Load\n");
		break;

	case SDLK_F11: // PC Keyboard mode, thanks to zanny
	    if (ksym.mod & KMOD_ALT)
        {
            SetPCKeyboard(spcsys.RAM);
        }
        else
        {
            ToggleFullScreen();
        }
		break;
    case SDLK_KP5:
        spconf.debug = 1;
        break;
	case SDLK_F12: // Reset
        if (ksym.mod & KMOD_ALT)
        {
            spcsys.IPL_SW = 1;
            printf("Reset with IPL_SW\n");
        }
        else
        {
            spcsys.IPL_SW = 0;
            printf("Reset (keeping tape pos.)\n");
        }
		loadROM("spcall.rom");
		InitIOSpace();
		SndQueueInit();
//		SetMC6847Mode(SET_TEXTMODE, 0);
        spcsys.Z80R.ICount = I_PERIOD;
        spcsys.Z80R.PC.W = 0x00;
        //spcsys.Z80R.SP.W = 0xf000;
        spcsys.IPLK = 1;
        spcsys.IPL_SW = 1;
        z80mem = spcsys.ROM;
		spcsim.baseTick = SDL_GetTicks();
		spcsim.prevTick = spcsim.baseTick;
		spcsys.intrTime = INTR_PERIOD;
		spcsys.tick = 0;
        spcsys.refrTimer = 0;	// timer for screen refresh
        spcsys.refrSet = spconf.frameRate;	// init value for screen refresh timer
		ResetZ80(&spcsys.Z80R);
		break;
	}
}

/**
 * SDL Key-Down processing. Search Hash table and set appropriate keyboard matrix.
 * @param sym SDL key symbol
 */
void ProcessKeyDown(SDLKey sym)
{
	int i;
	int index = sym % 256;
    //printf(">%d-%c\n", sym);
	for (i = 0; i < KeyHashTab[index].numEntry; i++)
	{
		if (KeyHashTab[index].keys[i].sym == sym)
		{
			spcsys.keyMatrix[KeyHashTab[index].keys[i].keyMatIdx]
				&= ~(KeyHashTab[index].keys[i].keyMask);
#ifdef DEBUG_MODE
			printf("%08x [%s] key down\n",
				KeyHashTab[index].keys[i].sym, KeyHashTab[index].keys[i].keyName);
#endif
			break;
		}
	}
}

/**
 * SDL Key-Up processing. Search Hash table and set appropriate keyboard matrix.
 * @param sym SDL key symbol
 */
void ProcessKeyUp(SDLKey sym)
{
	int i;
	int index = sym % 256;
    //printf("<%d-%c\n", sym);
	for (i = 0; i < KeyHashTab[index].numEntry; i++)
	{
		if (KeyHashTab[index].keys[i].sym == sym)
		{
			spcsys.keyMatrix[KeyHashTab[index].keys[i].keyMatIdx]
				|= (KeyHashTab[index].keys[i].keyMask);
#ifdef DEBUG_MODE
			printf("%08x [%s] key up\n",
				KeyHashTab[index].keys[i].sym, KeyHashTab[index].keys[i].keyName);
#endif
			break;
		}
	}
}


/**
 * Check if any keyboard event exists and process it
 */
void CheckKeyboard(void)
{
	SDL_Event	event ;	// SDL event

	while (SDL_PollEvent(&event) > 0)
	{
        if (m_uiStr)
        {
            if (event.key.keysym.sym == SDLK_BACKSPACE)
            {
                m_uiStr = NULL;
                continue;
            }
        }
		switch (event.type)
		{
		case SDL_KEYDOWN:
			ProcessSpecialKey(event.key.keysym);
			ProcessKeyDown(event.key.keysym.sym);
			break;
		case SDL_KEYUP:
			ProcessKeyUp(event.key.keysym.sym);
			break;
		case SDL_QUIT:
			printf("Quit requested, quitting.\n");
			exit(0);
			break;
        // case SDL_VIDEORESIZE:
            // SDL_Resize(event.resize);
            // break;
		case SDL_SYSWMEVENT:
			printf("WindowEvent=%0x08x\n", event.syswm.msg);
			break;
		}
	}
}

/**
 * Read and trash any remaining keyboard event
 */
void flushSDLKeys(void)
{
	SDL_Event	event ;	// SDL event
	int cnt = 0;

	while (SDL_PollEvent(&event) > 0)
		cnt++;
	printf("Total %d events flushed.\n");
}


/*************************************************************/
/** Misc. Callback Functions                                **/
/*************************************************************/
extern unsigned short breakpoint[];
extern int bp;

void CheckHook(register Z80 *R)
{
    unsigned short i = 0;
    int debug = 0;
    int bpcheck = 0;
    if (bp > 0)
    {
        for (i = 0; i < 20; i++)
        {
            if (breakpoint[i] != 0) {
                bpcheck++;
                if (R->PC.W == breakpoint[i] || R->Trace == 1)
                {
                    debug = 1;
                    break;
                }
            }
            if (bpcheck >= bp)
                break;
        }
    }
    if (debug || spconf.debug)
    {
        spconf.debug = DebugZ80(R);
    }
}

void PatchZ80(register Z80 *R)
{}

word LoopZ80(register Z80 *R)
{
	return INT_NONE;
}

void ShowCredit(void)
{
	printf("[SPC-1000 Emulator V%s]\n\n", SPC_EMUL_VERSION);
	printf("Written by ionique (K.-H. Sihn).\n");
	printf("Thanks to zanny, loderunner, zzapuno, kosmo, mayhouse.\n");
	printf("contact: http://blog.naver.com/ionique\n\n");

	printf("This emulator uses Z-80 and AY-3-8910 emulation from Marat Fayzullin.\n");
	printf("Find more about them at http://fms.komkon.org/\n\n");

	printf("Brief usage of keyboard:\n");
	printf("F7 : exit\n");
	printf("F8 : cassette PLAY button\n");
	printf("F9 : cassette REC button\n");
	printf("F10: cassette STOP button\n");
	printf("F11: PC-compatible keyboard layout (IOCS change, by zanny)\n");
	printf("F12: Reset (keeping tape position)\n");
	printf("Scroll Lock: Turbo mode\n");
	printf("TAB: LOCK key\n");
	printf("PgUp/PgDn: Save/Load current status\n\n");

	printf("For other settings, see SPCEMUL.INI\n");

	printf("This program is freeware, provided with no warranty.\n");
	printf("The author takes no responsibility for ");
	printf("any damage or legal issue\ncaused by using this program.\n");
}

SDL_Surface *vdpsf;

void DisplayUpdate(void)
{
	SDL_Rect rect, r;
	SDL_Surface *s;
#if 0
	rect.x = rect.y = 0;
	rect.w = spcsdl.w*2; rect.h = spcsdl.h*2;
	s = SDL_DisplayFormat(vdpsf);
    SDL_SoftStretch(s, NULL, spcsdl.emul, &rect);	
#else
	rect.x = r.x = 0;
	rect.w = spcsdl.w*2;
	r.w = spcsdl.w;
	rect.h = r.h = 1;
	s = SDL_DisplayFormat(vdpsf);
	for (int i = 0; i < 240; i++)
	{
		rect.y = i * 2; r.y = i;
		SDL_SoftStretch(s, &r, spcsdl.emul, &rect);
		rect.y +=1;
		SDL_FillRect(spcsdl.emul, &rect, SDL_MapRGB(spcsdl.emul->format,0, 0x33, 0));
	}
#endif	
	SDL_Flip( spcsdl.emul );
	SDL_FreeSurface(s);
}

void ToggleFullScreen()
{
    spconf.fullscreen = !spconf.fullscreen;
    SDL_SetVideoMode(spconf.rw, spconf.rh, spconf.bpp, spcsdl.flag | (spconf.fullscreen ? SDL_FULLSCREEN : 0));
}
void InitSDL()
{
    if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER)==-1)) {
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
        exit(-1);
    }
    if (TTF_Init() < 0)
    {
        printf("Could not initialize SDL_tff: %s", SDL_GetError());
        exit(-2);
    }
    int fbfd = 0;
    fbfd = open("/dev/fb0", O_RDWR);
	spcsdl.flag = SDL_HWSURFACE;
    spconf.rw = 640;
    spconf.rh = 480;
    spconf.dx = spcsdl.w = spconf.rw/2;
    spconf.dy = spcsdl.h = spconf.rh/2;
    spcsdl.rect.x = 0;
    spcsdl.rect.y = 0;
    spcsdl.rect.w = spconf.rw;
    spcsdl.rect.h = spconf.rh;
    spconf.bpp = 16;
    if (fbfd != -1) {
        const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
		spconf.dx = videoInfo->current_w;
		spconf.dy = videoInfo->current_h;
        spconf.bpp = videoInfo->vfmt->BitsPerPixel;
        spcsdl.flag |= SDL_FULLSCREEN;
        close(fbfd);
    }
//	if (spconf.fullscreen)
//		spcsdl.flag |= SDL_FULLSCREEN;
	SDL_WM_SetCaption("SPC-1000 emulator", NULL);
    spcsdl.emul = SDL_SetVideoMode(spconf.rw, spconf.rh, spconf.bpp, spcsdl.flag );
    if (!spcsdl.emul)
    {
        printf("SDL video initialization failed\n");
        exit(-1);
    }
	printf("Set %dx%d at %d bits-per-pixel mode\n", spconf.rw, spconf.rh,
           spcsdl.emul->format->BitsPerPixel);	
    return;
}

void SDL_Resize(SDL_ResizeEvent &resize)
{
    spconf.rw = resize.w;
    spconf.rh = resize.h;
    spcsdl.rect.x = (spconf.rw > spcsdl.rect.w ? (spconf.rw - spcsdl.rect.w) / 2 : 0);
    spcsdl.rect.y = (spconf.rh > spcsdl.rect.h ? (spconf.rh - spcsdl.rect.h) / 2 : 0);
    if (spcsdl.rect.w > spconf.rw) spcsdl.rect.w = spconf.rw; else spcsdl.rect.w = spcsdl.w;
    if (spcsdl.rect.h > spconf.rh) spcsdl.rect.h = spconf.rh; else spcsdl.rect.h = spcsdl.h;
    SDL_SetVideoMode(spconf.rw, spconf.rh, spconf.bpp, spcsdl.flag);
}
int ExecuteThread(void *);

void initDisk(void)
{
    FILE *f;
    if (strlen(spcsys.fdd.diskfile) > 4)
    {
        f = fopen(spcsys.fdd.diskfile, "rb");
		if (f > 0) {
        	fread(spcsys.fdd.diskdata, 1, sizeof(spcsys.fdd.diskdata), f);
	        fclose(f);
		}
    }
    if (strlen(spcsys.fdd.diskfile2) > 4)
    {
        f = fopen(spcsys.fdd.diskfile2, "rb");
		if (f > 0) {
        	fread(spcsys.fdd.diskdata2, 1, sizeof(spcsys.fdd.diskdata2), f);
	        fclose(f);
		}
    }
}

/**
 * Starting point and main loop for SPC-1000 emulator
 */
int main(int argc, char* argv[])
{
    SDL_Thread *thread;
    int         threadReturnValue;
	FILE *fp;	// for reading ROM file
	//freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
	loadROM("spcall.rom");
    //memcpy(spcsys.ROM + 32768, spcsys.ROM, 32768);
	z80mem = spcsys.ROM;
    //ShowWindow (GetConsoleWindow(), SW_HIDE);
    strcpy(load_path_taps, "../cas");
    strcpy(path_disks, "../disk");
    strcpy(spconf.path_snaps, ".");
	ReadINI((char*)"SPCEMUL.INI");

	InitIOSpace();
	ShowCredit();
	InitUIPaste();
	InitSDL();
	initDebug();
	fb = InitMC6847(); // Tells VRAM address to MC6847 module and init
	t99 = tms9918_create();
	tms9918_framebuffer((unsigned char *)malloc(256*192), 256, 192);
	vdpsf = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 8, 0,0,0,0);
    printf("format-%d(%d,%d,%d,%d)\n", vdpsf->format->BytesPerPixel, vdpsf->format->Rmask, vdpsf->format->Gmask, vdpsf->format->Bmask, vdpsf->format->Amask);
    //printf("w=%d, h=%d, t=%d, l=%d, r=%d, b=%d\n", vdpsf->w, vdpsf->h);
	
	fb = (PIXEL *)vdpsf->pixels;
	SDL_Color *pal;
	if (vdpsf != 0 && vdpsf->format->palette != NULL)
	{
		pal = (SDL_Color *) malloc(sizeof(SDL_Color) * 256);
		pal[0] = (SDL_Color) _CLR0;
		pal[1] = (SDL_Color) _CLR1;
		pal[2] = (SDL_Color) _CLR2;
		pal[3] = (SDL_Color) _CLR3;
		pal[4] = (SDL_Color) _CLR4;
		pal[5] = (SDL_Color) _CLR5;
		pal[6] = (SDL_Color) _CLR6;
		pal[7] = (SDL_Color) _CLR7;
		pal[8] = (SDL_Color) _CLR8;
		pal[9] = (SDL_Color) _CLR9;
		pal[10] = (SDL_Color) _CLR10;
		pal[11] = (SDL_Color) _CLR11;
		pal[12] = (SDL_Color) _CLR12;
		pal[13] = (SDL_Color) _CLR13;
		pal[14] = (SDL_Color) _CLR14;
		SDL_SetColors(vdpsf, pal, 0, 14);
		SDL_SetPalette(vdpsf, SDL_LOGPAL, pal, 0, 14);
	}
	else
	{
		printf("exit(0)");
		exit(0);
	}
	printf("framebuffer=%x\n", fb);
	printf("SDLSurface=%x\n", vdpsf);
	init_menu();
	initDisk();
	OpenSoundDevice();
	BuildKeyHashTab();	// Init keyboard hash table
	spcsim.baseTick = SDL_GetTicks();
	spcsim.prevTick = spcsim.baseTick;
	spcsys.intrTime = INTR_PERIOD;
	spcsys.tick = 0;
	spcsys.refrTimer = 0;	// timer for screen refresh
	spcsys.refrSet = spconf.frameRate;	// init value for screen refresh timer

    if (spconf.pcKeyboard > 0)
    {
        //SetPCKeyboard(spcsys.ROM);
    }
#define THREAD	
#ifdef _THREAD
    thread = SDL_CreateThread(ExecuteThread, NULL);
    SDL_WaitThread(thread, &threadReturnValue);
#else
    ExecuteThread(0);
#endif // THREAD
}
int ExecuteThread(void *data)
{
	int prevTurboState = 0;
	int turboState = 0;
	int tick = 0;
	int count = 0;
	t = timeGetTime();
	Z80 *R = &spcsys.Z80R;		// Z-80 register
	ResetZ80(R);
	R->ICount = I_PERIOD;
	spcsys.cycles = 0;
	spcsys.tick = 0;
	Uint8 *t99fb = video_get_vbp(0);
	t99->regs[0] = 2;
	t99->regs[1] = 0x40;
	t99->regs[2] = 14;
	t99->regs[3] = 1;
	t99->regs[4] = 7;
	Uint8 *name = t99->memory + ((t99->regs[2] & 0x0f) << 10);
	Uint8 *cdata = t99->memory + ((t99->regs[3] & 0x80)? 0x2000: 0);
	Uint8 *pdata = t99->memory + ((t99->regs[4] & 0x04)? 0x2000: 0);
	printf("nametable:%04x, pdata=%04x, cdata=%04x\n", name-t99->memory, pdata-t99->memory, cdata-t99->memory);
	for(int i=0;i < 256;i++)
	{
		name[i] = name[256+i] = name[512+i] = i;
	}
	while (1)	// Main emulation loop
	{

		if (spconf.debug || bp > 0)
            CheckHook(R);	// Special Processing, if needed

		if (R->ICount <= 0)	// 1 msec counter expired
		{
			tick++;		// advance spc tick by 1 msec
			spcsys.tick += (TURBO + 1);
			R->ICount += I_PERIOD;//_TURBO;	// re-init counter (including compensation)

			if (tick % 26 == 0)	// 1/60 sec interrupt timer expired
			{
				if (R->IFF & IFF_EI)	// if interrupt enabled, call Z-80 interrupt routine
				{
					R->IFF |= IFF_IM1 | IFF_1;
					IntZ80(R, 0);
				}
			}
			if (tick % 33 == 0 && t99)			// check refresh timer
			{
				SDL_LockSurface(vdpsf);
				Update6847(spcsys.GMODE, &spcsys.VRAM[0], fb);
				Update9918(spcsys.GMODE, &spcsys.VRAM[0], pdata, cdata);
				do {
					tms9918_periodic(t99);
				} while(t99->scanline);
				for(int i = 0; i < 192; i++)
					for(int j = 0; j < 256; j++)
						fb[j+(320-256)/2+320*(i+(240-192)/2)] = cMap[t99fb[i*256+j]];
//				printf("1");
				fflush(stdout);
				SDL_UnlockSurface(vdpsf);
				DisplayUpdate();
				CheckKeyboard();
			}
			Loop8910(&spcsys.ay8910, 1);
			int d = timeGetTime() - t;
			if (d > 100) 
				t = timeGetTime(); 
			else
			{
				while(t>timeGetTime() && !TURBO);
				t++;
			}
		}
		count = R->ICount;
		ExecZ80(R); // Z-80 emulation
		spcsys.cycles += (count - R->ICount);
	}
	CloseMC6847();
	return 0;
}

