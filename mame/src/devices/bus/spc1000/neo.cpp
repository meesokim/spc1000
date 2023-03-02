// license:BSD-3-Clause
// copyright-holders:Miso Kim meeso.kim@gmail.com
/***************************************************************************

    SPC-1000 neo expansion unit

***************************************************************************/

#include <map>
#include <string>
#include <fstream>
using namespace std;

// #ifndef __SPCBOX__
// #define __SPCBOX__

// #define rATN (1<<7)
// #define rDAC (1<<6)
// #define rRFD (1<<5)
// #define rDAV (1<<4)
// #define wDAC (1<<2)
// #define wRFD (1<<1)
// #define wDAV (1<<0)

// enum {
//     SDINIT,
//     SDWRITE,
//     SDREAD,
//     SDSEND,
//     SDCOPY,
//     SDFORMAT,
//     SDSTATUS,
//     SDDRVSTS,
//     SDRAMTST,
//     SDTRANS2,
//     SDNOACT,
//     SDTRANS1,
//     SDRCVE,
//     SDGO,
//     SDLOAD,
//     SDSAVE,
//     SDLDNGO,
//     RPI_FILES=0x20,
//     RPI_LOAD,
//     RPI_SAVE,
//     RPI_OLDNUM,
// };

// #include <map>
// #include "miniz.h"
// using namespace std;
// #ifdef _WIN32
// #define strcasestr strcasecmp
// #include <experimental/filesystem>
// namespace fs = std::experimental::filesystem;
// #else
// #include <filesystem>
// // namespace fs = std::filesystem;
// #endif
// #include <unistd.h>
// // #include <tinyexr>
// using std::map;
// using std::string;
// // using tinyexr::miniz;
// class TapeFiles {
//     int len = 0;
//     int skipdir = 0;
//     int pos = 0;
//     int fsize = 0;
//     int fileno;
//     uint32_t ttime=0;
//     char tapename[2048];
//     // char *files[4096];
//     // uint32_t ttime = 0;
//     map<int, const char *>files;
//     bool zipped = false;
//     bool cas = false;
//     mz_zip_archive Zip;
//     int zpos = 0;
//     char bpos = 0;
//     char data[1024*1024*4];
//     char rbuf[8192];
// public:
//     FILE *rfp = 0, *wfp = 0;
//     int motor;			// Motor Status
//     int pulse;			// Motor Pulse (0->1->0) causes motor state to flip
//     TapeFiles(const char *ext) {
//         fileno = 0;
//         if (strcasestr(ext, ".zip")) {
//             zipped = true;
//             memset(&Zip, 0, sizeof(mz_zip_archive));
//             mz_zip_reader_init_file(&Zip, ext, 0);
//             int nums = (int)mz_zip_reader_get_num_files(&Zip);
//             for (len = 0; len < nums; len++) {
//                 char *fname = new char[2048];
//                 mz_zip_reader_get_filename(&Zip, len, fname, 2048);
//                 files.insert(map<int, const char *>::value_type(len, fname));
//             }
//         } else {
//             zipped = false;
//             skipdir = strlen(ext) + 1;
// #if 0
//             for (const auto & entry : fs::directory_iterator(ext))
//             {
//     #ifdef _WIN32
//                 std::wstring wstr = entry.path().c_str();
//                 std::string str;
//                 str.assign(wstr.begin(), wstr.end());
//     #else
//                 std::string str = entry.path().c_str();
//     #endif
//                 const char * ext0 = str.substr(str.size()-4).c_str();
//                 if (strcasestr(ext0, ".tap") || strcasestr(ext0, ".cas")) {
//                     char *filename = new char[str.size()+1];                
//                     strcpy(filename, str.c_str());
//                     // files[len++] = filename; 
//                     files.insert(map<int, const char *>::value_type(len++, filename));
//                     // printf("%d.%s\n", len, filename+skipdir);
//                     // m.insert(map<int, string>::value_type(x, tapename));
//                 }
//             }
// #endif
//         }
//     }

//     void initialize() {
//         if (rfp)
//             fclose(rfp);
//         rfp = 0;
//         fsize = 0;
//         zpos = 0;
//     }

//     const char *filelist() {
//         static char filelist[16384];
//         const char *filename, *tmp;
//         int len = 0, fl = 0;
//         for (auto& file:files) {
//             filename = (const char*)file.second;
//             tmp = strchr(filename, '/');
//             if (tmp)
//                 filename = tmp + 1;
//             fl = strlen(filename);
//             fl = min(fl, 24);
//             strncpy(filelist+len, filename, (fl > 24 ? 24 : fl));
//             len += fl + 1;
//             *(filelist + len - 1) = '\\';
//         }
//         printf("\nfiles:%s\n", filelist);
//         return (const char *)filelist;
//     }
//     void prev() {
//         if (len) {
//             if (fileno > 0)
//                 fileno--;
//             else 
//                 fileno = len - 1;
//             strcpy(tapename, files[fileno] + skipdir);
//             if (rfp)
//                 fclose(rfp);
//             rfp = 0;
//             fsize = 0;
//             // ttime = timeGetTime();
//             // printf("%d.%s\n", fileno, tapename);
//         }
//     }

//     void next() {
//         if (len) {
//             if (fileno < len - 1)
//                 fileno++;
//             else
//                 fileno = 0;
//             strcpy(tapename, files[fileno] + skipdir);
//             if (rfp)
//                 fclose(rfp);
//             rfp = 0;
//             fsize = 0;
//             // ttime = timeGetTime();
//             // printf("%d.%s\n", fileno, tapename);
//         }
//     }

//     // const char* getTapeName() {
//     //     static char str[4096];
//     //     if (timeGetTime() - ttime < 3000)
//     //     {   
//     //         sprintf(str, "%d.%s", fileno, tapename);
//     //         return str;
//     //     }
//     //     return "";
//     // }

//     const char *filename() {
//         return tapename;
//     }

//     int load(int num) {
//         load(files[fileno = num]);  
//         return fsize;
//     }

//     void load() {
//         load(files[fileno]);        
//     }
//     void load(const char *file) {
//         if (zipped) {
//             strcpy(tapename, file);
//             mz_zip_reader_get_filename (&Zip, fileno, tapename, 2048);
//             size_t uncomp_size;
//             // mz_zip_reader_extract_file_to_mem_no_alloc(&Zip, tapename, data, sizeof(data), 0, rbuf, sizeof(rbuf));
//             char *p = (char *)mz_zip_reader_extract_file_to_heap(&Zip, tapename, &uncomp_size, 0);
//             memcpy(data, p, uncomp_size);
//             free(p);
//             fsize = uncomp_size;
//             // mz_zip_archive_file_stat stat;
//             // mz_zip_reader_file_stat(&Zip, fileno, &stat);
//             // fsize = (int) stat.m_uncomp_size;
//             // file = stat.m_filename;
//             zpos = 0;
//             // printf("filename:%s, size:%d\n", tapename, fsize);
//         } else {
//             rfp = fopen(file, "r");
//             // printf("file load: %s %x\n", file, (uint32_t)(uint64_t)rfp);
//             strcpy(tapename, file);
//             fseek(rfp, 0L, SEEK_END);
//             fsize = ftell(rfp);
//             // fseek(fp, 0L, SEEK_SET);
//             rewind(rfp);
//             fread(data, 1, fsize, rfp);
//             fclose(rfp);
//             rfp = 0;
//             zpos = 0;
//         }
//         if (strcasestr(file, ".cas")) {
//             zpos = 16;
//             cas = true;
//             // printf("cas file: %s\n", file);
//         } else {
//             cas = false;
//         }
//         printf("filename:%s(%d)\n", file, fsize);
//     }

//     char getc() {
//         char c = '0';
//         if (len) {
//             if (zpos > fsize) {
//                 load();
//             } else {
//                 // ttime = timeGetTime();
//                 if (cas) {
//                     c = '0' + ((data[zpos] >> (7 - bpos++)) & 1);
//                     if (bpos > 7)
//                     {
//                         zpos++;
//                         bpos = 0;
//                     }
//                 }
//                 else
//                     c = data[zpos++];
//                 // printf("%c", c, zpos);
//             }
//         } 
//         return c;
//     }

//     // bool time() {
//     //     return timeGetTime() - ttime < 3000;
//     // }

//     void putc(char c) {

//     }

//     int percent() {
//         if (fsize > 0 && motor) {
//             return zpos*100/fsize;
//         }
//         return -1;
//     }
// };

// #define TAPEFILES

// class SpcBox {
//   private:
//     // #include "spc1000.inc"
//     uint8_t *spc1000_bin;
//     bool exe_req = false;
//     uint8_t datain, dataout, direct_value, status, cnt;
//     uint8_t drv, blocks, tracks, sectors;
//     uint8_t *fdd[3], *writebuf, *rpibuf;
//     uint8_t params[10];
//     uint32_t p, q, bsize, rsize, oldnum, fno, rpi_idx;
//     uint8_t rdskbuf[256*16*80], buffer[1024*1024], wdiskbuf[256*16*80];
//     map<uint8_t, string>m, cmds;
//     string drive, pattern;
// #ifdef THREAD
//     std::vector<std::thread> threads;
//     thread t;
// #endif
//     TapeFiles tape{"tap.zip"};
//   public:
//     SpcBox() {
//         m[0] = "CLEAR";
//         m[rATN] = "ATN";
//         m[wRFD] = m[rRFD] = "RFD";
//         m[wDAV] = m[rDAV] = "DAV";
//         m[wDAC] = m[rDAC] = "DAC";
//         cmds[SDINIT] = "SDINIT";
//         cmds[SDREAD] = "SDREAD";
//         cmds[SDSTATUS] = "SDSTATUS";
//         cmds[SDSEND] = "SDSEND";
//         cmds[SDDRVSTS] = "SDDRVSTS";
//         cmds[RPI_FILES] = "RPI_FILES";
//         cmds[RPI_LOAD] = "RPI_LOAD";
//         cmds[RPI_SAVE] = "RPI_SAVE";
//         cmds[RPI_OLDNUM] = "RPI_OLDNUM";
//         drive = "SD:/";
//         pattern = "*.tap";
//         cnt = 0;
//         initialize();
//         ifstream is( "spc1000.bin", std::ios::binary );
// 		is.seekg(0, is.end);
// 		int length = (int)is.tellg();
// 		is.seekg(0, is.beg);        
//         spc1000_bin = (unsigned char*)malloc(length);
//         is.read((char*)spc1000_bin, length);
// 		is.close();
//     } 
//     void initialize() {
//         bsize = direct_value = p = q = 0;
//         fdd[0] = (uint8_t *)spc1000_bin;
//         status = 0;
//         dataout = 0;
//     }

//     const char * files() {
// #ifdef TAPEFILES
//         return tape.filelist();
// #else
//         const char * filelist = "1_PROTECTOR.tap\\2cas.tap\\9_PING PONG.tap\\BOAT-5239-mayhouse.tap\\CrushCircle-mayhouse.tap\\DisneyLand_adv0.tap\\DisneyLand_adv1.tap\\DisneyLand_adv2.tap\\DisneyLand_adv3.tap\\DisneyLand_adv4.tap\\DisneyLand_adv5.tap\\ET_Miro-mayhouse.tap\\GOLDCAVERN.tap\\GalagWars-mayhouse.tap\\GunFright.tap\\HeadOn-mayhouse.tap\\MiddleSchoolEnglish.tap\\PENGO.tap\\PHOENIX.tap\\SNAKE-9009-mayhouse.tap\\Sigrape2.tap\\SpaceGang-mayhouse.tap\\VDP-zanac-mayhouse.tap\\Wizerdy.tap\\[]kangaroo.tap\\a.v-6856-mayhouse.tap\\adventure.tap\\apple thief.tap\\apple.tap\\basic.tap\\baveque-mayhouse.tap\\block-2788-mayhouse.tap\\boot.tap\\computerorgan.tap\\d.tap\\dasm & rlct.tap\\ddd.tap\\demo.tap\\disassembler.tap\\egg_catch_v1.1-mayhouse.\\fighter 201.tap\\firia-1146-mayhouse.tap\\flyboat.tap\\for.tap\\goonies-mayhouse.tap\\hangul.tap\\hats-5678-mayhouse.tap\\icbm.tap\\kangaroo.tap\\keenon.tap\\kingsvalley.tap\\led.tap\\led1.tap\\lode-runner1-mayhouse.ta\\lode_runner-mayhouse.tap\\lode_runner1-mayhouse.ta\\lupan_4-mayhouse.tap\\mini organ.tap\\miro2-1041-mayhouse.tap\\morse.tap\\orion.tap\\othello.tap\\overwater-826-mayhouse.t\\penzerspiche-4135-mayhou\\penzerspitze-4135-mayhou\\protector.tap\\radation.tap\\rambo.tap\\red ball.tap\\relocater 1_1.tap\\relocater.tap\\scramble-mayhouse.tap\\sd720.tap\\sd725.tap\\sinpanufo.tap\\smba.tap\\smbb.tap\\spacemission.tap\\spc1500_demo.tap\\styx monitor.tap\\sub routine.tap\\superxevious-mayhouse.ta\\tank2-7698-mayhouse.tap\\wawa.tap\\xevious.tap\\z80assem.tap\\zexas-5461-mayhouse.tap\\������+��\\42column.cas\\9-16_mevious-mtwtfss365.\\Deep-scan+(1984)+(static\\Flappy.cas\\Jet+set+willy.cas\\Mevious+(198x)+(-).cas\\TUTANCANMEN.cas\\Tom+&+Jerry.cas\\VDP-Castle(key+bug).cas\\VDP-Castle+excellent.cas\\VDP-Knightmare.cas\\Xevious.cas\\cassette_voice7ca0-mayho\\dang_goo-mayhouse.cas\\knight_lore-mayhouse.cas\\la_pulce.cas\\lunar_city[b]-mayhouse.c\\lunar_city[m]-mayhouse.c\\miracle_world-mayhouse.c\\roadwoker-mayhouse.cas\\skypannic-mayhouse.cas\\toyar-4852-mayhouse.cas";
//         return filelist;
// #endif
//     }
//     void load(int num) {
// #ifdef TAPEFILES
//         bsize = tape.load(num);
// #else
//         bsize = 0;
// #endif
//     }
//     void execute() {
//         execute(exe_req);
//     }
//     void execute(bool exe_req) {
//         if (!exe_req)
//             return;
//         // printf("execute\n");
//         switch(params[0]) {
//             case SDINIT:
//                 buffer[0] = 100;
//                 bsize = 1;
//                 break;
//             case SDWRITE:
//                 if (p == 4)
//                 {
//                     blocks = params[1];
//                     drv = params[2];
//                     tracks = params[3];
//                     sectors = params[4];
//                     writebuf = fdd[drv] + (tracks * 16 + (sectors - 1))*256;
//                 } else if (p > 4)
//                 {
//                     writebuf[p - 5] = datain;
//                 }
//                 break;
//             case SDREAD:
//                 if (p == 4) 
//                 {
//                     blocks = params[1];
//                     drv = params[2];
//                     tracks = params[3];
//                     sectors = params[4];												
//                     rsize = 256 * blocks;
//                     memcpy(rdskbuf, fdd[drv]+(tracks * 16 + (sectors - 1))*256, rsize);
//                     printf("\n%dbytes (drive:%d, tracks:%d, sectors:%d, blocks:%d)\n", rsize, drv, tracks, sectors, blocks);
//                     // cout << cmds[params[0]] << blocks << "," << drv << "," << tracks << "," << sectors << "," << rsize << " executed" << endl;
//                 }
//                 break;
//             case SDSEND:
//                 memcpy(buffer, rdskbuf, rsize);
//                 bsize = rsize;
//                 break;
//             case SDCOPY:
//                 break;
//             case SDFORMAT:
//                 break;
//             case SDSTATUS:
//                 buffer[0] = 0xc0;
//                 bsize = 1;
//                 break;
//             case SDDRVSTS:
//                 buffer[0] = 0xff;
//                 bsize = 1;
//                 break;
//             case RPI_FILES:
//                 if (p == 0)
//                 {
//                     rpi_idx = 0;
//                     rpibuf = (uint8_t *)pattern.c_str();
//                 } 
//                 if (datain == 0)
//                 {
//                     strcpy((char *)buffer, files());
//                     bsize = strlen((char *)buffer) + 1;
//                 }
//                 else if (params[p] == '\\')
//                 {
//                     rpibuf[rpi_idx] = 0;
//                     rpi_idx = 0;
//                     strcpy((char *)rpibuf, pattern.c_str());
//                 }
//                 else
//                     rpibuf[rpi_idx++] = datain;
//                 break;
//             case RPI_LOAD:
//                 if (p == 2)
//                 {
//                     oldnum = fno = params[1] + params[2] * 256;
//                     printf("num=%d\n", oldnum);
//                     load(oldnum);
//                 }
//                 break;
//             case RPI_OLDNUM:
//                 buffer[0] = oldnum & 0xff;
//                 buffer[1] = oldnum >> 8;
//                 printf("oldnum=%d", oldnum);
//                 bsize = 2;
//                 q = 0;
//                 break;								
//         }
//         if (bsize > 0) {
//             printf("\n");
//         }
//         p++;
//         status |= wDAC;
//         this->exe_req = false;
//     }
//     uint8_t read(uint8_t addr) {
//         uint8_t ret = 0;
//         // static uint8_t d;
//         string cmd[4] = {"", "GETDATA", "STATUS", "DIRECT"};
//         switch (addr) {
//             case 0:
//                 ret = cnt;
//                 break;
//             case 1: // get data
//                 ret = dataout;
//                 // printf("%02x(%c)\n", ret, ret);
//                 break;
//             case 2: // status
//                 ret = status;
//                 break;
//             case 3: // direct input
//                 ret = direct_value;
//                 break;
//         }
// // #define DEBUG
// #ifdef DEBUG        
//         if (addr == 2 && d != ret) {
//             c = '\n';
//             d = ret;
//         }
//         else
//             c = '\r';
//         if (addr == 2){
//             string str;
//             str = status & 4 ? m[4] : "";
//             if (status & 2)
//                 str += str.length() ? "|" + m[2] : m[2];
//             if (status & 1)
//                 str += str.length() ? "|" + m[1] : m[1];
//             if (!str.length())
//                 str = "CLEAR";
//             printf("%s(%02x):%s\t\t\t%c", cmd[addr&3].c_str(), ret, str.c_str(), c);
//         }
//         else
//             printf("%s(%02x):%02x\t\t\t%c", cmd[addr&3].c_str(), addr, ret, c);
// #endif
//         return ret;
//     };
//     void write(uint8_t addr, uint8_t data) {
//         switch (addr) {
//             case 0: // output data
//                 datain = data;
//                 // printf("datain:%02x\n", datain);
//                 break;
//             case 1:
//                 cnt = data;
//                 break;
//             case 3: // direct access clock for direct input
//                 // if (q < bsize - 1) {
// #ifdef TAPEFILES
//                     direct_value = tape.getc() - '0';
//                     // printf("%c", direct_value + '0');
// #else
//                     direct_value = 0;
// #endif
//                     q++;
//                 // }
//                 break;
//             case 2: // status check
//                 // printf("out(%02x):%s          \n", addr, m[data].c_str());
//                 if (data == rATN) {
//                     p = 0;
//                     status = wRFD;
//                 } else if (data == rRFD) {
//                     // status &= ~wRFD;
//                     status |= wDAV;
//                     dataout = buffer[q];
//                 } else if (data == rDAC) {
//                     status &= ~wDAV;
//                     if (q < bsize - 1)
//                         q++;                    
//                 } else if (data == rDAV) {
//                     status &= ~wDAC;
//                     if (p < sizeof(params) - 1)
//                         params[p] = datain;
//                     q = 0;
//                     if (p == 0) {
//                         printf("%s(%d):", cmds[datain].c_str(), datain);
//                         bsize = 0;
//                     } else {
//                         printf("%02x,", datain);
//                     }
// #ifdef THREAD                    
//                     threads.push_back(thread([this] { this->execute(this); }));
//                     for (auto& t:threads)
//                         if (t.joinable()) {
//                             t.join();
//                         }
// #endif                        
//                     exe_req = true;
//                     dataout = 0;
//                 } else if (!data){
//                     if (status & wDAC) {
//                         status &=~wDAC;
//                     }
//                     if (status & wDAV) {
//                         status &=~wDAV;
//                     }
//                 }
//                 break;
//         }
//     };
// };
// #endif 
#include "spcbox.h"

#include "emu.h"
#include "neo.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

// WRITE_LINE_MEMBER(spc1000_neo_exp_device::vdp_interrupt)
// {
// 	// nothing here?
// }

//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void spc1000_neo_exp_device::device_add_mconfig(machine_config &config)
{
	// TMS9928A(config, m_vdp, XTAL(10'738'635)); // TODO: which clock?
	// m_vdp->set_vram_size(0x4000);
	// m_vdp->int_callback().set(FUNC(spc1000_neo_exp_device::vdp_interrupt));
	// m_vdp->set_screen("tms_screen");
	// SCREEN(config, "tms_screen", SCREEN_TYPE_RASTER);
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SPC1000_NEO_EXP, spc1000_neo_exp_device, "spc1000_neo_exp", "SPC1000 VDP expansion")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

SpcBox *sbox = 0;
//-------------------------------------------------
//  spc1000_neo_exp_device - constructor
//-------------------------------------------------

spc1000_neo_exp_device::spc1000_neo_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPC1000_NEO_EXP, tag, owner, clock)
	, device_spc1000_card_interface(mconfig, *this)
{
    if (!::sbox)
        ::sbox = new SpcBox();
    sbox=::sbox;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spc1000_neo_exp_device::device_start()
{
	sbox->initialize();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spc1000_neo_exp_device::device_reset()
{
}

/*-------------------------------------------------
    read
-------------------------------------------------*/
uint8_t spc1000_neo_exp_device::read(offs_t offset)
{
	if (offset > 0xff)
		return 0xff;
	return sbox->read(offset);
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void spc1000_neo_exp_device::write(offs_t offset, uint8_t data)
{
	if (offset <= 0xff)
	{
		sbox->write(offset, data);
        sbox->execute();
	}
}
