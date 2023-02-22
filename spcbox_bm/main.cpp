#include <stdint.h>
#include <cstring>
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

#define SDINIT		0
#define SDWRITE		1
#define SDREAD		2
#define SDSEND  	3
#define SDCOPY		4
#define SDFORMAT	5
#define SDSTATUS	6
#define SDDRVSTS	7
#define SDRAMTST	8
#define SDTRANS2	9
#define SDNOACT		10
#define SDTRANS1	11
#define SDRCVE		12
#define SDGO		13
#define SDLOAD		14
#define SDSAVE		15
#define SDLDNGO		16
#define RPI_FILES	0x20
#define RPI_LOAD	0x21
#define RPI_OLDNUM	0x23

#define READY 			0
#define READ_FOR_DATA  	1
#define DATA_VALID 		2
#define RECEIVED		3
#define rATN 0x80
#define rDAC 0x40
#define rRFD 0x20
#define rDAV 0x10
#define wDAC 0x04
#define wRFD 0x02
#define wDAV 0x01

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

int main(void) {
    uint32_t readsize;
    uint8_t datain, dataout, data3, data0, cflag, blocks, drv, tracks, sectors;
    char *tmpbuf, *rpibuf;
    int params[10], p, q, t, rpi_idx, len, len0, fileno;
    static char diskbuf[258*256*8], buffer[256*256*32], tapbuf[256*256*32];
    char * fdd[3];
    //int f[256] = {0x11,0x7,0xcb,0xcd,0xf3,0x07,0xc9,0x48,0x65,0x6c,0x6c,0x20,0x77,0x6f,0x72,0x6c,0x64,0x21,0x00};	
    // char f[256*256];
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
    GPIO_SET(0x0);
    // PUT32(ARM_GPIO_GPFEN0, RPSPC_EXT);
	// asm("cpsid i");
	// asm("cpsie f");
    // PUT32(ARM_IC_FIQ_CONTROL, 0x80 | ARM_FIQ_GPIO0);
    data0 = 1;
    int a = 0;
    while(true) {
        a = GPIO_GET();
        if (!(a & RPSPC_EXT))
        {
            volatile register uint8_t addr = (a & (RPSPC_A0 | RPSPC_A1)) >> RPSPC_A0_PIN;
            // volatile register uint8_t data = GPIO_GET();
            if (a & RPSPC_WR) {
                GPIO_CLR(0xff);
                switch(addr) {
                    case 0:
                        GPIO_SET(data0++);
                        break;
                    case 1:
                        GPIO_SET(dataout);
                        // dataout = 0;
                        break;
                    case 2:
                        GPIO_SET(cflag);
                        break;
                    case 3:
                        GPIO_SET(data3);
                        break;
                }
            } else {
                switch(addr) {
                    case 0:
                        datain = GPIO_GET();
                        break;
                    case 3:
                        data3 = tapbuf[t++] == '1' ? 1 : 0;	
                        break;
                    case 2:
                        switch(GPIO_GET() & 0xf0) {
                            case rATN: // 0x80 --> 0x02
                                cflag = wRFD;
                                p = 0;
                                break;
                            case rRFD: // 0x20 --> 0x01
                                cflag |= wDAV;
                                break;
                            case rDAC: // 0x40 --> 0x00
                                if (cflag & wDAV)
                                {
                                    cflag &= ~wDAV;
                                    q++;
                                }
                                break;
                            case rDAV: // 0x10 --> 0x04
                                if (!(cflag & wDAC))
                                {
                                    cflag |= wDAC;
                                    if (p < 10)
                                        params[p] = datain;
                                    q = 0;
                                    execute = true;
                                }									
                                break;
                            case 0: // 0x00 --> 0x00
                                if (cflag & wDAC)
                                {
                                    cflag &= ~wDAC;
                                    p++;
                                }
                                else if (cflag & wDAV)
                                {
                                    dataout = buffer[q];
                                }
                                break;                            
                            default:
                                break;
                    }
                }
            }               
            while(!(GPIO_GET() & RPSPC_EXT));
        }
        if (execute) {
            switch (params[0])
            {
                case SDINIT:
                    buffer[0] = 100;
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
                    // Message.Format ("SDSEND(%d) %02x %02x %02x\n", p, buffer[0],  buffer[1], buffer[2]);
                    // m_Screen.Write ((const char *) Message, Message.GetLength ());
                    break;
                case SDCOPY:
                    memcpy(fdd[params[5]]+(params[6] * 16 + (params[7]-1))*256, fdd[params[2]]+(params[3]*16+(params[4]-1))*256, 256 * params[1]);
                    break;
                case SDSTATUS:
                    buffer[0] = 0xc0;
                    // Message.Format ("SDSTATUS\n");
                    // m_Screen.Write ((const char *) Message, Message.GetLength ());
                    break;
                case ((int)SDDRVSTS):
                    buffer[0] = 0xff;
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
            execute = false;
        }					
    }
}