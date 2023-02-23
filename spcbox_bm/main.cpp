#include <stdint.h>
#include <cstring>
#include <cstdio>
#include "pireg.h"

#define RPSPC_D0	(1 << 0)
#define MD00_PIN	0
#define RPSPC_WR	(1 << 14)
#define RPSPC_EXT	(1 << 17)
#define RPSPC_RST   (1 << 27)
#define RPSPC_CLK	(1 << 18)
#define RPSPC_A0	(1 << 22)
#define RPSPC_A1	(1 << 23)
#define RPSPC_A11	(1 << 24)
#define RPSPC_A6	(1 << 25)
#define RPSPC_A0_PIN 22

#define ADDR  (RPSPC_A0 | RPSPC_A1)
#define ADDR0 0
#define ADDR1 RPSPC_A0
#define ADDR2 RPSPC_A1

// #define SDINIT		0
// #define SDWRITE		1
// #define SDREAD		2
// #define SDSEND  	3
// #define SDCOPY		4
// #define SDFORMAT	5
// #define SDSTATUS	6
// #define SDDRVSTS	7
// #define SDRAMTST	8
// #define SDTRANS2	9
// #define SDNOACT		10
// #define SDTRANS1	11
// #define SDRCVE		12
// #define SDGO		13
// #define SDLOAD		14
// #define SDSAVE		15
// #define SDLDNGO		16
// #define RPI_FILES	0x20
// #define RPI_LOAD	0x21
// #define RPI_OLDNUM	0x23

#define READY 			0
#define READ_FOR_DATA  	1
#define DATA_VALID 		2
#define RECEIVED		3

#include "fatfs/ff.h"

#define DRIVE		"SD:"

char files[256*256];
char files2[256*256];
char drive[256];
char pattern[256];

char * fnRPI_FILES(char *drive, char *pattern)
{
	// Show contents of root directory
	DIR Directory;
	FILINFO FileInfo;
	char FileName[256];
	FRESULT Result = f_findfirst (&Directory, &FileInfo,  DRIVE "/", "*.cas");
	int len = 0, len2= 0, length = 0;
	memset(files, 0, 256*256);
	memset(files2, 0, 256*256);
	for (unsigned i = 0; Result == FR_OK && FileInfo.fname[0]; i++)
	{
		if (!(FileInfo.fattrib & (AM_HID | AM_SYS)))
		{
			strcpy(files+len, FileInfo.fname);
			strcpy(files2+len2, FileInfo.fname);
			length = strlen(FileInfo.fname);
			len += length;
			len2 += length > 24 ? 24 : length;
			*(files+(len++))='\\';
			*(files2+(len2++))='\\';
		}

		Result = f_findnext (&Directory, &FileInfo);
	}
	// FileName.Format ("%s\n", files);
	// m_Screen.Write ((const char *) FileName, FileName.GetLength ());			
	return files;
}

char *strrchr(const char *s, int ch)
{
    char *start = (char *) s;

    while (*s++);
    while (--s != start && *s != (char) ch);
    if (*s == (char) ch) return (char *) s;

    return 0;
}



extern "C"
{
    #include "spc1000.h"
	__attribute__((interrupt("FIQ"))) interrupt_fiq(uint32_t a) {};
	// {
	// 	// volatile register uint32_t a = GPIO_GET();
    //     volatile register uint8_t addr = (a & (RPSPC_A0 | RPSPC_A1)) >> RPSPC_A0_PIN;
    //     volatile register bool rd = (a & RPSPC_WR);
    //     volatile register uint8_t data = a;
    //     if (rd) {
    //         GPIO_CLR(0xff);
    //         switch(addr) {
    //             case 0:
    //                 GPIO_SET(data0++);
    //                 break;
    //             case 1:
    //                 GPIO_SET(dataout);
    //                 dataout = 0;
    //                 break;
    //             case 2:
    //                 GPIO_SET(cflag);
    //                 break;
    //             case 3:
    //                 GPIO_CLR(0xff);
    //                 GPIO_SET(data3);
    //                 break;
    //         }
    //     } else {
    //         switch(addr) {
    //             case 0:
    //                 datain = data;
    //                 break;
    //             case 3:
    //                 data3 = tapbuf[t++] == '1' ? 1 : 0;	
    //                 break;
    //             case 2:
    //                 switch(data & 0xf0) {
    //                     case rATN: // 0x80 --> 0x02
    //                         cflag = wRFD;
    //                         p = 0;
    //                         break;
    //                     case rRFD: // 0x20 --> 0x01
    //                         cflag |= wDAV;
    //                         break;
    //                     case rDAC: // 0x40 --> 0x00
    //                         if (cflag & wDAV)
    //                         {
    //                             cflag &= ~wDAV;
    //                             q++;
    //                         }
    //                         break;
    //                     case 0: // 0x00 --> 0x00
    //                         if (cflag & wDAC)
    //                         {
    //                             cflag &= ~wDAC;
    //                             p++;
    //                         }
    //                         else if (cflag & wDAV)
    //                         {
    //                             dataout = buffer[q];
    //                         }
    //                         break;                            
    //                     case rDAV: // 0x10 --> 0x04
    //                         if (!(cflag & wDAC))
    //                         {
    //                             cflag |= wDAC;
    //                             if (p < 10)
    //                                 params[p] = datain;
    //                             q = 0;
    //                             execute = true;
    //                         }									
    //                         break;
    //                     default:
    //                         break;
    //             }
    //         }
    //     }               
    //     while(!(GPIO_GET() & RPSPC_EXT));
    //     PUT32(ARM_GPIO_GPEDS0, RPSPC_EXT);        
    // }
    extern int mount(const char *source);    
}

#define rATN (1<<7)
#define rDAC (1<<6)
#define rRFD (1<<5)
#define rDAV (1<<4)
#define wDAC (1<<2)
#define wRFD (1<<1)
#define wDAV (1<<0)

// #include <thread>
using namespace std;

#include <map>
#include <vector>

#include <iostream>

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
    RPI_OLDNUM,
};

// #define printf(fmt, ...) (0)
class SpcBox {
  private:

    #include "spc1000.inc"
    bool exe_req = false;
    uint8_t datain, dataout, direct_value, status;
    uint8_t drv, blocks, tracks, sectors;
    uint8_t *fdd[3], *writebuf, *rpibuf;
    uint8_t params[10];
    uint32_t p, q, bsize, rsize, oldnum, fno, rpi_idx;
    uint8_t rdskbuf[256*16*80], buffer[1024*1024], wdiskbuf[256*16*80];
#ifdef THREAD    
    std::vector<std::thread> threads;
    thread t;
#endif
    map<uint8_t, string>m, cmds;
    string drive, pattern;
  public:
    SpcBox() {
        bsize = direct_value = p = q = 0;
        fdd[0] = (uint8_t *)spc1000_bin;
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
        cmds[RPI_OLDNUM] = "RPI_OLDNUM";
        drive = "SD:/";
        pattern = "*.tap";
    }

    const char * files() {
        // return sys.tape.filelist();
        return 0;
    }
    void load(int num) {
        // bsize = sys.tape.load(num);
        bsize = 0;
    }
    void execute(SpcBox *sbox) {
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
                    rsize = 256 * blocks;
                    memcpy(rdskbuf, fdd[drv]+(tracks * 16 + (sectors - 1))*256, rsize);
                    printf("%s: %dbytes (drive:%d, tracks:%d, sectors:%d, blocks:%d)\n", cmds[params[0]].c_str(), rsize, drv, tracks, sectors, blocks);
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
                if (datain == 0)
                {
                    strcpy((char *)buffer, files());
                    bsize = strlen((char *)buffer) + 1;
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
                    load(oldnum);
                }
                break;
            case RPI_OLDNUM:
                buffer[0] = oldnum & 0xff;
                buffer[1] = oldnum >> 8;
                q = 0;
                break;								
        }
        if (!p) {
            printf("%s\t\t\n", cmds[params[0]].c_str());
        }
        p++;
        status |= wDAC;
        exe_req = false;
    }
    uint8_t read(uint8_t addr) {
        uint8_t ret, c;
        static uint8_t a, d;
        string cmd[4] = {"", "GETDATA", "STATUS", "DIRECT"};
        switch (addr) {
            case 1: // get data
                ret = dataout;
                // printf("%02x(%c)\n", ret, ret);
                break;
            case 2: // status
                ret = status;
                break;
            case 3: // direct input
                ret = direct_value;
                break;
        }
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
        switch (addr) {
            case 0: // output data
                datain = data;
                printf("datain:%02x\n", datain);
                break;
            case 3: // direct access clock for direct input
                if (q < bsize - 1) {
                    // direct_value = sys.tape.getc() - '0';
                    direct_value = 0;
                    q++;
                }
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
                    printf("param[%d]=%02x\n", p, datain);
#ifdef THREAD                    
                    threads.push_back(thread([this] { this->execute(this); }));
                    for (auto& t:threads)
                        if (t.joinable()) {
                            t.join();
                        }
#else
                    execute(this);
#endif                        
                    exe_req = true;
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
    ~SpcBox() {
        // for(int i=0;i<threads.size();i++)
        //     threads[i].join();
    }
};

int main(void) {
    uint32_t readsize;
    uint8_t datain, dataout, data3, data0, cflag, blocks, drv, tracks, sectors;
    char *tmpbuf, *rpibuf;
    uint8_t params[10], p, q, t, rpi_idx, len, len0, fileno;
    static char diskbuf[258*256*32], buffer[256*256*32], tapbuf[256*256*32];
    char * fdd[3];
    char filename[256];
    static int oldnum = 0;    
    bool wr;
    int cmd = 0;
    FRESULT Result;
    bool execute = false;    
	mount("SD:");
    fdd[0] = spc1000_bin;
    rpibuf = 0;
    rpi_idx = 0;
    datain = 0;
    dataout = 0;
    blocks = cflag = p = q = t = data0 = data3 = 0;
    readsize = 0;
    tmpbuf = 0;        
	GPIO_SEL0(IOSEL0);
    GPIO_SEL1(0);
    GPIO_SEL2(0);
    GPIO_CLR(0xff);
    // PUT32(ARM_GPIO_GPFEN0, RPSPC_EXT);
	// asm("cpsid i");
	// asm("cpsid f");
    // PUT32(ARM_IC_FIQ_CONTROL, 0x80 | ARM_FIQ_GPIO0);
    data0 = 0;
    int a = 0;
    int bsize = 0;
    while(true) {
        a = GPIO_GET();
        if (!(a & RPSPC_EXT))
        {
            volatile register uint8_t addr = (a >> RPSPC_A0_PIN) & 3;
            if (a & RPSPC_WR) {
                GPIO_CLR(0xff);
                switch(addr) {
                    case 0: 
                        GPIO_SET(execute);
                        break;
                    case 1:
                        GPIO_SET(dataout);
                        break;
                    case 2:
                        GPIO_SET(cflag);
                        break;
                    case 3:
                        GPIO_SET(data3);
                        break;
                }
            } else {
                // GPIO_SET(0xff);
                // volatile register uint8_t data = a;
                switch(addr) {
                    case 0:
                        datain = a;
                        break;
                    case 3:
                        data3 = tapbuf[t++] == '1' ? 1 : 0;	
                        break;
                    case 2:
                        switch(a & 0xf0) {
                            case rATN: // 0x80 (ATN=1) --> 0x02 (RFD=1) COMMAND
                                p = 0;
                                cflag = wRFD;
                                break;
                            case rRFD: // 0x20 (RFD=1) --> 0x01 (DAV=1) GETDATA
                                cflag &= ~wRFD;
                                cflag |= wDAV;
                                dataout = buffer[q];
                                break;
                            case rDAC: // 0x40 (DAC=1) --> 0x00 (DAV=0) GETDATA confirm
                                cflag &= ~wDAV;
                                if (q < bsize - 1)
                                    q++;
                                break;
                            case rDAV: // 0x10 (DAV=1) --> 0x04 (DAC=1) SENDDATA confirm
                                cflag &= ~wDAC;
                                params[p] = datain;
                                q = 0;
                                execute = true;
                                dataout = 0;
                                break;
                            default:
                                cflag &= ~wDAV;
                                cflag &= ~wDAC;
                                break;
                        }   
                    default:
                        break;
                }
            }               
            while(!(GPIO_GET() & RPSPC_EXT));
        }
        else if (!(a & RPSPC_RST)) {
            dataout = 0;
            datain = 0;
            cflag = 0;
            p = 0;
            q = 0;
            GPIO_CLR(0xff);
            while(!(GPIO_GET() & RPSPC_RST));
        }
        else if (execute) {
            switch (params[0])
            {
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
                        tmpbuf = fdd[drv] + (tracks * 16 + (sectors - 1))*256;
                    } else if (p > 4)
                    {
                        tmpbuf[p - 5] = datain;
                    }
                    break;
                case SDREAD:
                    if (p == 4) 
                    {
                        blocks = params[1];
                        drv = params[2];
                        tracks = params[3];
                        sectors = params[4];												
                        readsize = 256 * blocks;
                        memcpy(diskbuf, fdd[drv]+(tracks * 16 + (sectors - 1))*256, readsize);
                    }
                    break;
                case SDSEND:
                    memcpy(buffer, diskbuf, readsize);
                    bsize = readsize;
                    // q = 0;
                    // Message.Format ("SDSEND(%d) %02x %02x %02x\n", p, buffer[0],  buffer[1], buffer[2]);
                    // m_Screen.Write ((const char *) Message, Message.GetLength ());
                    break;
                case SDCOPY:
                    memcpy(fdd[params[5]]+(params[6] * 16 + (params[7]-1))*256, fdd[params[2]]+(params[3]*16+(params[4]-1))*256, 256 * params[1]);
                    break;
                case SDSTATUS:
                    buffer[0] = 0xc0;
                    bsize = 0;
                    // Message.Format ("SDSTATUS\n");
                    // m_Screen.Write ((const char *) Message, Message.GetLength ());
                    break;
                case SDDRVSTS:
                    buffer[0] = 0xff;
                    bsize = 0;
                    // Message.Format ("SDDRVSTS\n");
                    // m_Screen.Write ((const char *) Message, Message.GetLength ());
                    break;
                case RPI_FILES:
                    if (p == 0)
                    {
                        rpi_idx = 0;
                        strcpy(drive, "SD:/");
                        strcpy(pattern, "*.tap");
                        rpibuf = drive;
                    } 
                    if (datain == 0)
                    {
                        if (rpibuf == pattern || p == 0)
                        {
                            tmpbuf = fnRPI_FILES(drive, pattern);
                            strcpy(buffer, files2);
                            // Message.Format ("RPI_FILES: drive=%s, pattern=%s\n%s\n", drive, pattern, files2);
                            // m_Screen.Write ((const char *) Message, Message.GetLength ());
                        }
                    }
                    else if (params[p] == '\\')
                    {
                        rpibuf[rpi_idx] = 0;
                        rpi_idx = 0;
                        rpibuf = pattern;
                    }
                    else
                        rpibuf[rpi_idx++] = datain;
                    break;
                case RPI_LOAD:
                    if (p == 2)
                    {
                        oldnum = fileno = params[1] + params[2] * 256;
                        // Message.Format ("fileno:%d\n", fileno);
                        // m_Screen.Write ((const char *) Message, Message.GetLength ());											
                        len0 = 0;
                        len = 0;
                        while(len0 < fileno)
                        {
                            if (files[len++] == '\\')
                                len0++;
                        }	
                        len0 = 0;
                        while(files[len+len0++] != '\\'); 
                        memcpy(filename, "SD:/", 5);
                        memcpy(filename+4, files+len, len0);
                        *(filename+4+len0-1)=0;
                        FILINFO fno;
                        FIL File;
                        f_stat(filename, &fno);
                        // Message.Format ("RPI_LOAD: No.%d %s (%d)\n", fileno, filename, fno.fsize);
                        // m_Screen.Write ((const char *) Message, Message.GetLength ());
                        Result = f_open (&File, filename, FA_READ | FA_OPEN_EXISTING);
                        if (Result != FR_OK)
                        {
                            // FileName.Format ("loading failed: %s\n", filename);
                            // m_Screen.Write ((const char *) FileName, FileName.GetLength ());
                        }
                        else
                        {
                            unsigned nBytesRead;
                            char *point;
                            if((point = strrchr(filename,'.')) != 0 ) {
                                if(strcmp(point,".cas") == 0) {
                                    f_read(&File, buffer, fno.fsize, &nBytesRead);
                                    for(unsigned i = 15; i < nBytesRead; i++)
                                    {
                                        tapbuf[i*8]   = ((buffer[i] >> 7) & 1) + '0';
                                        tapbuf[i*8+1] = ((buffer[i] >> 6) & 1) + '0';
                                        tapbuf[i*8+2] = ((buffer[i] >> 5) & 1) + '0';
                                        tapbuf[i*8+3] = ((buffer[i] >> 4) & 1) + '0';
                                        tapbuf[i*8+4] = ((buffer[i] >> 3) & 1) + '0';
                                        tapbuf[i*8+5] = ((buffer[i] >> 2) & 1) + '0';
                                        tapbuf[i*8+6] = ((buffer[i] >> 1) & 1) + '0';
                                        tapbuf[i*8+7] = ((buffer[i] >> 0) & 1) + '0';
                                    }
                                }
                            }
                            else														
                                f_read(&File, tapbuf, fno.fsize, &nBytesRead);
                            f_close (&File);
                            // FileName.Format ("loading successful: %s\n", filename);
                            // m_Screen.Write ((const char *) FileName, FileName.GetLength ());
                            t = 0;
                        }
                    }
                    break;
                case RPI_OLDNUM:
                    buffer[0] = oldnum & 0xff;
                    buffer[1] = oldnum >> 8;
                    q = 0;
                    break;								
                default:
                    buffer[0] = cmd * cmd;
                    break;
            }
            cflag |= wDAC;
            execute = false;
            p++;
        }					
    }
}