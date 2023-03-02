#ifndef __SPCBOX__
#define __SPCBOX__

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
    RPI_OLDNUM,
};

class SpcBox {
  private:
    #include "spc1000.inc"
    bool exe_req = false;
    uint8_t datain, dataout, direct_value, status, cnt;
    uint8_t drv, blocks, tracks, sectors;
    uint8_t *fdd[3], *writebuf, *rpibuf;
    uint8_t params[10];
    uint32_t p, q, bsize, rsize, oldnum, fno, rpi_idx;
    uint8_t rdskbuf[256*16*80], buffer[1024*1024], wdiskbuf[256*16*80];
    map<uint8_t, string>m, cmds;
    string drive, pattern;
#ifdef THREAD
    std::vector<std::thread> threads;
    thread t;
#endif
  public:
    SpcBox() {
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
        cnt = 0;
        initialize();
    } 

    void initialize() {
        bsize = direct_value = p = q = 0;
        fdd[0] = (uint8_t *)spc1000_bin;
        status = 0;
        dataout = 0;
    }

    const char * files() {
        // return sys.tape.filelist();
        string filelist = "1_PROTECTOR.tap\\2cas.tap\\9_PING PONG.tap\\BOAT-5239-mayhouse.tap\\CrushCircle-mayhouse.tap\\DisneyLand_adv0.tap\\DisneyLand_adv1.tap\\DisneyLand_adv2.tap\\DisneyLand_adv3.tap\\DisneyLand_adv4.tap\\DisneyLand_adv5.tap\\ET_Miro-mayhouse.tap\\GOLDCAVERN.tap\\GalagWars-mayhouse.tap\\GunFright.tap\\HeadOn-mayhouse.tap\\MiddleSchoolEnglish.tap\\PENGO.tap\\PHOENIX.tap\\SNAKE-9009-mayhouse.tap\\Sigrape2.tap\\SpaceGang-mayhouse.tap\\VDP-zanac-mayhouse.tap\\Wizerdy.tap\\[]kangaroo.tap\\a.v-6856-mayhouse.tap\\adventure.tap\\apple thief.tap\\apple.tap\\basic.tap\\baveque-mayhouse.tap\\block-2788-mayhouse.tap\\boot.tap\\computerorgan.tap\\d.tap\\dasm & rlct.tap\\ddd.tap\\demo.tap\\disassembler.tap\\egg_catch_v1.1-mayhouse.\\fighter 201.tap\\firia-1146-mayhouse.tap\\flyboat.tap\\for.tap\\goonies-mayhouse.tap\\hangul.tap\\hats-5678-mayhouse.tap\\icbm.tap\\kangaroo.tap\\keenon.tap\\kingsvalley.tap\\led.tap\\led1.tap\\lode-runner1-mayhouse.ta\\lode_runner-mayhouse.tap\\lode_runner1-mayhouse.ta\\lupan_4-mayhouse.tap\\mini organ.tap\\miro2-1041-mayhouse.tap\\morse.tap\\orion.tap\\othello.tap\\overwater-826-mayhouse.t\\penzerspiche-4135-mayhou\\penzerspitze-4135-mayhou\\protector.tap\\radation.tap\\rambo.tap\\red ball.tap\\relocater 1_1.tap\\relocater.tap\\scramble-mayhouse.tap\\sd720.tap\\sd725.tap\\sinpanufo.tap\\smba.tap\\smbb.tap\\spacemission.tap\\spc1500_demo.tap\\styx monitor.tap\\sub routine.tap\\superxevious-mayhouse.ta\\tank2-7698-mayhouse.tap\\wawa.tap\\xevious.tap\\z80assem.tap\\zexas-5461-mayhouse.tap\\������+��\\42column.cas\\9-16_mevious-mtwtfss365.\\Deep-scan+(1984)+(static\\Flappy.cas\\Jet+set+willy.cas\\Mevious+(198x)+(-).cas\\TUTANCANMEN.cas\\Tom+&+Jerry.cas\\VDP-Castle(key+bug).cas\\VDP-Castle+excellent.cas\\VDP-Knightmare.cas\\Xevious.cas\\cassette_voice7ca0-mayho\\dang_goo-mayhouse.cas\\knight_lore-mayhouse.cas\\la_pulce.cas\\lunar_city[b]-mayhouse.c\\lunar_city[m]-mayhouse.c\\miracle_world-mayhouse.c\\roadwoker-mayhouse.cas\\skypannic-mayhouse.cas\\toyar-4852-mayhouse.cas\\";
        return filelist.c_str();
    }
    void load(int num) {
        // bsize = sys.tape.load(num);
        bsize = 0;
    }
    void execute() {
        execute(exe_req);
    }
    void execute(bool exe_req) {
        if (!exe_req)
            return;
        printf("execute\n");
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
        this->exe_req = false;
    }
    uint8_t read(uint8_t addr) {
        uint8_t ret, c;
        static uint8_t a, d;
        string cmd[4] = {"", "GETDATA", "STATUS", "DIRECT"};
        switch (addr) {
            case 0:
                ret = cnt;
                break;
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
        switch (addr) {
            case 0: // output data
                datain = data;
                printf("datain:%02x\n", datain);
                break;
            case 1:
                cnt = data;
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
};
#endif 