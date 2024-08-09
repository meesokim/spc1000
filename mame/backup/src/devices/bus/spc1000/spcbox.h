#ifndef __SPCBOX__
#define __SPCBOX__

#include <map>
#include <string>
#include <unistd.h>
#include <algorithm>
// #ifndef RASPPI
#include <dirent.h>
// #endif
#include "miniz.h"
using std::map;
using std::string;
#define min(a, b) (a > b ? b : a)

#ifdef __circle__
#include "ob_file.h"
#define fopen ob_fopen
#define fseek ob_fseek
#define ftell ob_ftell
#define rewind ob_rewind
#define fread ob_fread
#define fclose ob_fclose
#endif
class TapeFiles {
    int len = 0;
    int skipdir = 0;
    int pos = 0;
    int fsize = 0;
    int fileno = 0;
    uint32_t ttime=0;
    char tapename[2048];
    // char *files[4096];
    // uint32_t ttime = 0;
    map<int, const char *>files;
    bool zipped = false;
    bool cas = false;
    mz_zip_archive Zip;
    int zpos = 0;
    char bpos = 0;
    char *data = 0;
    // char buf[1024*1024*4];
    // #include "tap.inc"
    char ext[1024];
    char rbuf[8192];
    char flist[1024*256];
    uint8_t buf[1024*1024*3];
    bool just_loaded = false;
public:
    FILE *rfp = 0, *wfp = 0;
    int motor;			// Motor Status
    int pulse;			// Motor Pulse (0->1->0) causes motor state to flip
    TapeFiles(const char *file = 0) {
        // printf("TapeFiles created:%s\n", file);
        memset(flist, 0, sizeof(flist));
        memset(buf, 0, sizeof(buf));
    }
    void initialize(const char *tapefiles[], int size) {
        printf("files#:%d\n", size);
        if (size == 1 && strcasestr(tapefiles[0], ".zip")) {
            initialize(tapefiles[0], 0);
        } else {
            for (int i=0; i<size; i++) {
                printf("%s\n", tapefiles[i]);
                files.insert(map<int, const char *>::value_type(len++, tapefiles[i]));
            }
        }
        makelist();
    }
    void initialize(const char *exp, int blen = 0) {
        strcpy(ext, exp);
        fileno = 0;
        printf("ext:%s\n", exp);
        if (blen) {
            zipped = true;
            memset(&Zip, 0, sizeof(mz_zip_archive));
            mz_zip_reader_init_mem(&Zip, exp, blen, 0);
            int nums = (int)mz_zip_reader_get_num_files(&Zip);
            for (len = 0; len < nums; len++) {
                char *fname = new char[2048];
                mz_zip_reader_get_filename(&Zip, len, fname, 2047);
                files.insert(map<int, const char *>::value_type(len, fname));
                // printf("%03d.%s\n", len, fname);
            }
            if (!len) {
                files.insert(map<int, const char*>::value_type(len++, "No tap or cas file"));
            }            
            printf("blen:%d\n", blen);
        }
        else if (strcasestr(ext, ".zip")) {
            printf("strcasestr:%s\n", ext);
            zipped = true;
            memset(&Zip, 0, sizeof(mz_zip_archive));
#if 1
            FILE *f = fopen(ext, "rb");
            fseek(f, 0, SEEK_END);
            int length = ftell(f);
            fseek(f, 0, SEEK_SET);
            blen = fread(buf, length, 1, f);
            printf("blen:%d\n", length);
            fclose(f);
            mz_zip_reader_init_mem(&Zip, buf, length, 0);
#else
#ifndef MINIZ_NO_STDIO
            mz_zip_reader_init_file(&Zip, ext, 0);
#endif
#endif
            int nums = (int)mz_zip_reader_get_num_files(&Zip);
            for (len = 0; len < nums; len++) {
                char *fname = new char[2048];
                mz_zip_reader_get_filename(&Zip, len, fname, 2047);
                files.insert(map<int, const char *>::value_type(len, fname));
                printf("%03d.%s\n", len, fname);
            }
        } else {
            printf("else:%s\n", ext);
            zipped = false;
#ifdef __LINUX__            
            if (!opendir(ext)) {
                files.insert(map<int, const char *>::value_type(len++, exp));
            } else {
                skipdir = strlen(ext) + 1;
                // struct stat tmp_stat; 
                dirent **list;
                printf("scandir:%s\n", ext);
                int count=scandir(ext, &list, NULL, alphasort);
                printf("dir:%s(%d)\n", ext, count);
                len = 0;
                int k = 0;
                while(count--)
                {
                    char *fname = new char[2048];
                    sprintf(fname, "%s/%s", ext, list[k]->d_name);
                    // printf("%d(%d).%s\n", k, len, fname);
                    if (strcasestr(fname, ".tap") || strcasestr(fname, ".cas"))
                    {
                        files.insert(map<int, const char *>::value_type(len++, fname));
                    }
                    k++;
                }    
            }
            printf("files:%s(%d)\n", ext, len);
#endif
        }
        if (!len) {
            files.insert(map<int, const char*>::value_type(len++, "No tap or cas file"));
        }
        makelist();
    }

    void initialize() {
        rfp = 0;
        fsize = 0;
        zpos = 0;
    }

    void makelist() {
        const char *filename;
        // int no = 0;
        printf("makelist\n");
        flist[0] = 0;
        int tlen = 0, llen = 0;
        for (int i = 0; i < len; i++) {
            filename = fileonly(files[i]);
            llen = min(strlen(filename), 26);
            strncpy(flist+tlen, filename, llen);
            strcat(flist, "\\\0");
            // printf("%s\n",flist);
            tlen += llen + 1;
        }
        printf("\nfiles:%s\n\n", flist);
    }
    const char *filelist() {
        return flist;
    }

    void updateTime() {
#ifdef TIMEGETTIME
            ttime = timeGetTime();
#endif
    }

    const char *fileonly(const char *filename) {
        printf("%s\n", filename);
        const char *tmp = filename + strlen(filename);                
        while(*tmp != '/' && tmp > filename) tmp--;
        if (*tmp == '/')
            tmp = tmp + 1;
        return tmp;
    }

    void prev() {
        if (len) {
            if (fileno > 0)
                fileno--;
            else 
                fileno = len - 1;
            if (rfp)
                fclose(rfp);
            rfp = 0;
            fsize = 0;
            updateTime();
            sprintf(tapename, "%d.%s", fileno, fileonly(files[fileno]));
        }
    }

    void next() {
        if (len) {
            if (fileno < len - 1)
                fileno++;
            else
                fileno = 0;
            if (rfp)
                fclose(rfp);
            rfp = 0;
            fsize = 0;
            updateTime();
            sprintf(tapename, "%d.%s", fileno, fileonly(files[fileno]));
        }
    }

    const char *getTapeName() {
        return tapename;
    }

	static char *
	strcasestr(const char *s, const char *find)
	{
		char c, sc;
		size_t len;
		if ((c = *find++) != 0) {
			c = (char)tolower((unsigned char)c);
			len = strlen(find);
			do {
				do {
					if ((sc = *s++) == 0)
						return (NULL);
				} while ((char)tolower((unsigned char)sc) != c);
			} while (strncasecmp(s, find, len) != 0);
			s--;
		}
		return ((char *)s);
	}
    
    static int
    strncasecmp(const char *s1, const char *s2, int length)
    {
        unsigned char u1, u2;

        for (; length != 0; length--, s1++, s2++) {
            u1 = (unsigned char) *s1;
            u2 = (unsigned char) *s2;
            if (u1 != u2) {
                return u1 - u2;
            }
            if (u1 == '\0') {
                return 0;
            }
        }
        return 0;
    }

	static char tolower(unsigned char c)
	{
		if ((c >= 'A') && (c <= 'Z'))
		{
			c = c - 'A' + 'a';
		}
		return c;
	}

    const char *filename() {
        return tapename;
    }

    int load(int num) {
        load(files[fileno = num]);  
        return fsize;
    }

    void load() {
        load(files[fileno]);        
    }
    void load(const char *file) {
        if (zipped) {
            strcpy(tapename, file);
            mz_zip_reader_get_filename (&Zip, fileno, tapename, 2048);
            size_t uncomp_size;
            // mz_zip_reader_extract_file_to_mem_no_alloc(&Zip, tapename, data, sizeof(data), 0, rbuf, sizeof(rbuf));
            char *p = (char *)mz_zip_reader_extract_file_to_heap(&Zip, tapename, &uncomp_size, 0);
            if (data)
                free(data);
            data = (char *)malloc(uncomp_size);
            memcpy(data, p, uncomp_size);
            free(p);
            fsize = uncomp_size;
            // mz_zip_archive_file_stat stat;
            // mz_zip_reader_file_stat(&Zip, fileno, &stat);
            // fsize = (int) stat.m_uncomp_size;
            // file = stat.m_filename;
            zpos = 0;
            // printf("filename:%s, size:%d\n", tapename, fsize);
        } else {
            rfp = fopen(files[fileno], "r");
            // strcpy(tapename, file);
            fseek(rfp, 0L, SEEK_END);
            fsize = ftell(rfp);
            fseek(rfp, 0L, SEEK_SET);
            if (data)
                free(data);
            data = (char *)malloc(fsize);
            size_t len = fread(data, fsize, 1, rfp);
            len++;
            fclose(rfp);
            // printf("file load: %s %x (%d)\n", file, (uint32_t)(uint64_t)rfp, fsize);
            rfp = 0;
            zpos = 0;
        }
        if (strcasestr(file, ".cas")) {
            zpos = 16;
            cas = true;
            // printf("cas file: %s\n", file);
        } else {
            cas = false;
        }
        printf("filename:%s(%d)\n", file, fsize);
        just_loaded = true;
    }

    char getc() {
        char c = '0';
        if (just_loaded) {
            just_loaded = false;
            return c;
        }
        if (len) {
            if (zpos > fsize) {
                zpos = bpos = 0;
                just_loaded = true;
            } else {
                updateTime();
                if (cas) {
                    c = '0' + ((data[zpos] >> (7 - bpos++)) & 1);
                    if (bpos > 7)
                    {
                        zpos++;
                        bpos = 0;
                    }
                }
                else
                    c = data[zpos++];
                // printf("%c", c, zpos);
            }
        } 
        return c;
    }

#ifdef TIMEGETTIME        
    bool time(int time = 3000) {
        return (timeGetTime() - ttime) < time;
    }
#endif
    void putc(char c) {

    }

    int percent() {
        if (fsize > 0 && motor) {
            return zpos*100/fsize;
        }
        return -1;
    }

    int length() {
        return len;
    }

};

#define TAPEFILES
#include <chrono>
#include <thread>
#include <condition_variable>
class SpcBox {
  private:
    #define rATN (1<<7)
    #define rDAC (1<<6)
    #define rRFD (1<<5)
    #define rDAV (1<<4)
    #define wDAC (1<<2)
    #define wRFD (1<<1)
    #define wDAV (1<<0)

    enum {
        SDINIT,
        SDWRITE,
        SDREAD,
        SDSEND,
        SDCOPY,
        SDFORMAT,
        SDSTATUS,
        SDDRVSTS,
        SDRAMTST,
        SDTRANS2,
        SDNOACT,
        SDTRANS1,
        SDRCVE,
        SDGO,
        SDLOAD,
        SDSAVE,
        SDLDNGO,
        RPI_FILES=0x20,
        RPI_LOAD,
        RPI_SAVE,
        RPI_OLDNUM,
    };
    static uint8_t spc1000_bin[];
    bool exe_req = false;
    uint8_t datain, dataout, direct_value, status, cnt;
    uint8_t drv, blocks, tracks, sectors;
    uint8_t *fdd[3], *writebuf, *rpibuf;
    uint8_t params[10];
    uint32_t p, q, bsize, rsize, oldnum, fno, rpi_idx;
    uint8_t rdskbuf[256*16*80], buffer[1024*1024], wdskbuf[256*16*80];
    map<uint8_t, string>m, cmds;
    string drive, pattern;
#ifdef THREAD
    std::condition_variable cv;
    std::mutex cv_m; 
    std::thread *th = 0;
#endif
  public:
#ifdef TAPEFILES
    TapeFiles *tape;
#endif
    SpcBox(TapeFiles *tape0 = 0) {
        if (tape0) {
            tape = tape0;
        }
        memset(rdskbuf, 0, sizeof(rdskbuf));
        memset(wdskbuf, 0, sizeof(wdskbuf));
        memset(buffer, 0, sizeof(buffer));
        m[0] = "CLEAR";
        m[rATN] = "ATN";
        m[wRFD] = m[rRFD] = "RFD";
        m[wDAV] = m[rDAV] = "DAV";
        m[wDAC] = m[rDAC] = "DAC";
        cmds[SDINIT] = "SDINIT";
        cmds[SDREAD] = "SDREAD";
        cmds[SDSTATUS] = "SDSTATUS";
        cmds[SDSEND] = "SDSEND";
        cmds[SDDRVSTS] = "SDDRVSTS";
        cmds[RPI_FILES] = "RPI_FILES";
        cmds[RPI_LOAD] = "RPI_LOAD";
        cmds[RPI_SAVE] = "RPI_SAVE";
        cmds[RPI_OLDNUM] = "RPI_OLDNUM";
        drive = "SD:/";
        pattern = "*.tap";
        direct_value = 0;
        cnt = 0;
        initialize();
        FILE *f;
        int ret;
        fdd[0] = spc1000_bin;
        f = fopen("number.txt","r");
        if (f) {
            ret = fscanf(f, "%d", &oldnum);
            ret++;
            fclose(f);
            if (tape->length() < oldnum) {
                oldnum = tape->length() - 1;
            }
        }
        else
            oldnum = 0;
    	printf("number:%d\n", oldnum);
    } 
    void initialize() {
        bsize = direct_value = p = q = 0;
        status = 0;
        dataout = 0;
#ifdef THREAD        
        if (!th) {
            th = new std::thread( [&]() { execute_thread(); });
            printf("thread created\n");
        }
#endif        
    }

    const char * files() {
#ifdef TAPEFILES
        return tape->filelist();
#else
        const char * filelist = "1_PROTECTOR.tap\\2cas.tap\\9_PING PONG.tap\\BOAT-5239-mayhouse.tap\\CrushCircle-mayhouse.tap\\DisneyLand_adv0.tap\\DisneyLand_adv1.tap\\DisneyLand_adv2.tap\\DisneyLand_adv3.tap\\DisneyLand_adv4.tap\\DisneyLand_adv5.tap\\ET_Miro-mayhouse.tap\\GOLDCAVERN.tap\\GalagWars-mayhouse.tap\\GunFright.tap\\HeadOn-mayhouse.tap\\MiddleSchoolEnglish.tap\\PENGO.tap\\PHOENIX.tap\\SNAKE-9009-mayhouse.tap\\Sigrape2.tap\\SpaceGang-mayhouse.tap\\VDP-zanac-mayhouse.tap\\Wizerdy.tap\\[]kangaroo.tap\\a.v-6856-mayhouse.tap\\adventure.tap\\apple thief.tap\\apple.tap\\basic.tap\\baveque-mayhouse.tap\\block-2788-mayhouse.tap\\boot.tap\\computerorgan.tap\\d.tap\\dasm & rlct.tap\\ddd.tap\\demo.tap\\disassembler.tap\\egg_catch_v1.1-mayhouse.\\fighter 201.tap\\firia-1146-mayhouse.tap\\flyboat.tap\\for.tap\\goonies-mayhouse.tap\\hangul.tap\\hats-5678-mayhouse.tap\\icbm.tap\\kangaroo.tap\\keenon.tap\\kingsvalley.tap\\led.tap\\led1.tap\\lode-runner1-mayhouse.ta\\lode_runner-mayhouse.tap\\lode_runner1-mayhouse.ta\\lupan_4-mayhouse.tap\\mini organ.tap\\miro2-1041-mayhouse.tap\\morse.tap\\orion.tap\\othello.tap\\overwater-826-mayhouse.t\\penzerspiche-4135-mayhou\\penzerspitze-4135-mayhou\\protector.tap\\radation.tap\\rambo.tap\\red ball.tap\\relocater 1_1.tap\\relocater.tap\\scramble-mayhouse.tap\\sd720.tap\\sd725.tap\\sinpanufo.tap\\smba.tap\\smbb.tap\\spacemission.tap\\spc1500_demo.tap\\styx monitor.tap\\sub routine.tap\\superxevious-mayhouse.ta\\tank2-7698-mayhouse.tap\\wawa.tap\\xevious.tap\\z80assem.tap\\zexas-5461-mayhouse.tap\\������+��\\42column.cas\\9-16_mevious-mtwtfss365.\\Deep-scan+(1984)+(static\\Flappy.cas\\Jet+set+willy.cas\\Mevious+(198x)+(-).cas\\TUTANCANMEN.cas\\Tom+&+Jerry.cas\\VDP-Castle(key+bug).cas\\VDP-Castle+excellent.cas\\VDP-Knightmare.cas\\Xevious.cas\\cassette_voice7ca0-mayho\\dang_goo-mayhouse.cas\\knight_lore-mayhouse.cas\\la_pulce.cas\\lunar_city[b]-mayhouse.c\\lunar_city[m]-mayhouse.c\\miracle_world-mayhouse.c\\roadwoker-mayhouse.cas\\skypannic-mayhouse.cas\\toyar-4852-mayhouse.cas";
        return filelist;
#endif
    }
    void load(int num) {
#ifdef TAPEFILES
        bsize = tape->load(num);
#else
        bsize = 0;
#endif
    }
#ifdef THREAD
    bool execute_thread() {
        while(true) {
            std::unique_lock<std::mutex> lk(cv_m);
            cv.wait(lk, [this]{return this->exe_req == true;});
            execute(exe_req);
        }
    }
#endif
    bool execute() {
        return execute(exe_req);
    }
    bool execute(bool exe_req) {
        if (!exe_req)
            return false;
        switch(params[0]) {
            case SDINIT:
                buffer[0] = 100;
                bsize = 1;
                break;
            case SDWRITE:
                if (p == 4)
                {
                    blocks = params[1];
                    drv = params[2];
                    tracks = params[3];
                    sectors = params[4];
                    writebuf = fdd[drv] + (tracks * 16 + (sectors - 1))*256;
                } else if (p > 4)
                {
                    writebuf[p - 5] = datain;
                }
                break;
            case SDREAD:
                if (p == 4) 
                {
                    blocks = params[1];
                    drv = params[2];
                    tracks = params[3];
                    sectors = params[4];												
                    rsize = 256 * (blocks+1);
                    memcpy(rdskbuf, fdd[drv]+(tracks * 16 + (sectors - 1))*256, rsize);
                    printf("\n%dbytes (drive:%d, tracks:%d, sectors:%d, blocks:%d)\n", (int) rsize, drv, tracks, sectors, blocks);
                    // cout << cmds[params[0]] << blocks << "," << drv << "," << tracks << "," << sectors << "," << rsize << " executed" << endl;
                }
                break;
            case SDSEND:
                memcpy(buffer, rdskbuf, rsize);
                bsize = rsize;
                break;
            case SDCOPY:
                break;
            case SDFORMAT:
                break;
            case SDSTATUS:
                buffer[0] = 0xc0;
                bsize = 1;
                break;
            case SDDRVSTS:
                buffer[0] = 0xff;
                bsize = 1;
                break;
            case RPI_FILES:
                if (p == 0)
                {
                    rpi_idx = 0;
                    rpibuf = (uint8_t *)pattern.c_str();
                } 
                else if (datain == 0)
                {
                    strcpy((char *)buffer, files());
                    bsize = strlen((char *)buffer) + 1;
                    printf("\n%s\n", buffer);
                }
                else if (params[p] == '\\')
                {
                    rpibuf[rpi_idx] = 0;
                    rpi_idx = 0;
                    strcpy((char *)rpibuf, pattern.c_str());
                }
                else
                    rpibuf[rpi_idx++] = datain;
                break;
            case RPI_LOAD:
                if (p == 2)
                {
                    oldnum = fno = params[1] + params[2] * 256;
                    bsize = tape->load(oldnum);
                    printf("num=%d, size=%d\n", oldnum, bsize);
                    char number[100];
                    FILE *f = fopen("number.txt","w");
                    sprintf(number, "%d\n", oldnum);
                    fwrite(number, strlen(number)+1, 1, f);
                    fclose(f);
                }
                break;
            case RPI_OLDNUM:
                buffer[0] = oldnum & 0xff;
                buffer[1] = oldnum >> 8;
                printf("oldnum=%d", oldnum);
                bsize = 2;
                q = 0;
                break;								
        }
        if (bsize > 0) {
            printf("\n");
        }
        p++;
        status |= wDAC;
        this->exe_req = false;
        return true;
    }
    uint8_t read(uint8_t addr) {
        uint8_t ret = 0;
        // static uint8_t d;
        string cmd[4] = {"", "GETDATA", "STATUS", "DIRECT"};
        switch (addr) {
            case 0:
                ret = datain;
                break;
            case 1: // get data
                ret = dataout;
                // if (params[0] >= 0x20) 
                //     printf("%c", ret);
                break;
            case 2: // status
                ret = status;
                break;
            case 3: // direct input
                ret = direct_value;
                break;
        }
// #define DEBUG
#ifdef DEBUG        
        if (addr == 2 && d != ret) {
            c = '\n';
            d = ret;
        }
        else
            c = '\r';
        if (addr == 2){
            string str;
            str = status & 4 ? m[4] : "";
            if (status & 2)
                str += str.length() ? "|" + m[2] : m[2];
            if (status & 1)
                str += str.length() ? "|" + m[1] : m[1];
            if (!str.length())
                str = "CLEAR";
            printf("%s(%02x):%s\t\t\t%c", cmd[addr&3].c_str(), ret, str.c_str(), c);
        }
        else
            printf("%s(%02x):%02x\t\t\t%c", cmd[addr&3].c_str(), addr, ret, c);
#endif
        return ret;
    };
    void write(uint8_t addr, uint8_t data) {
        // printf("sbox%d:%02x\n", addr, data);        
        switch (addr) {
            case 0: // output data
                datain = data;
                // printf("datain:%02x\n", datain);
                break;
            case 1:
                cnt = data;
                break;
            case 3: // direct access clock for direct input
#ifdef TAPEFILES
                    // printf("%lx, bsize=%d\n", (uint64_t)tape, bsize);
                    if (bsize > 100)
                        direct_value = tape->getc() - '0';
                    // printf("%c", direct_value + '0');
#else
                    direct_value = 0;
#endif
                    // q++;
                break;
            case 2: // status check
                // printf("out(%02x):%s          \n", addr, m[data].c_str());
                if (data == rATN) {
                    p = 0;
                    status = wRFD;
                } else if (data == rRFD) {
                    // status &= ~wRFD;
                    status |= wDAV;
                    dataout = buffer[q];
                } else if (data == rDAC) {
                    status &= ~wDAV;
                    if (q < bsize - 1)
                        q++;                    
                } else if (data == rDAV) {
                    status &= ~wDAC;
                    if (p < sizeof(params) - 1)
                        params[p] = datain;
                    q = 0;
                    if (p == 0) {
                        printf("%s(%d):", cmds[datain].c_str(), datain);
                        bsize = 0;
                    } else {
                        printf("%02x,", datain);
                    }
                    exe_req = true;
#ifdef THREAD                    
                    cv.notify_all();
#else
                    execute();
#endif
                    dataout = 0;
                } else if (!data){
                    if (status & wDAC) {
                        status &=~wDAC;
                    }
                    if (status & wDAV) {
                        status &=~wDAV;
                    }
                }
                break;
        }
    };
};

uint8_t SpcBox::spc1000_bin[6266] = {
#include "spc1000.bin.inc"
};

#endif 
