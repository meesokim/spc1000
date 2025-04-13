#ifndef AY8910_H
#define AY8910_H

extern "C" {
#include "emu2149.h"
};

#define PSG_CLOCK_RATE 44100
#define PSG_QUALITY_HIGH 0

class AY8910
{
    PSG *psg;
    static const int BUFFMASK = 0x1fff;
    int16_t buf[BUFFMASK];
    int16_t bbuf1[BUFFMASK];
    int16_t bbuf2[BUFFMASK];
    int pos = 0;
    int cpos = 0;
    int prev = 0;
    int16_t *BUF[2]={bbuf1, bbuf2};
public:
    uint8_t reg;
    AY8910(uint32_t clk = 4000000, uint32_t rate = PSG_CLOCK_RATE) 
    {
        psg = PSG_new(clk, rate);
        // setVolumeMode(1);
        // PSG_setClockDivider(psg, 1);
        set_quality(PSG_QUALITY_HIGH);
    }
    ~AY8910() { PSG_delete (psg); }
    void initTick(int tick)
    {
        prev = tick;
    }
    void update(int tick) {
        for (int i = 0; i < (tick - prev) * PSG_CLOCK_RATE / 1000; i++)
        {
            buf[pos++] = calc();
            if (pos > BUFFMASK) pos = 0;
        }
        prev = tick;
    }
    void pushbuf(int16_t *buff, int len)
    {
        for(int i = 0; i < len; i++)
        {
            buff[i] = buf[cpos++];
            if (cpos > BUFFMASK) cpos = 0;
        }
    }
    int16_t* copybuf(int len)
    {
        static bool bufs = false;
        bufs = !bufs;
        for(int i = 0; i < len; i++)
        {
            BUF[bufs][i] = buf[cpos++];
            if (cpos > BUFFMASK) cpos = 0;
        }
        return BUF[bufs];
    }
    void set_quality (uint32_t q) { PSG_set_quality(psg, q); }
    void set_rate (uint32_t r) { PSG_set_rate(psg, r); }
    void reset () { PSG_reset(psg);}
    void latch (uint8_t val) { reg = val; }
    void write (uint8_t val) { PSG_writeReg(psg, reg, val); }
    uint8_t read () { return readReg(reg); }
    void writeReg (uint32_t reg, uint32_t val) { PSG_writeReg(psg, reg, val); }
    void writeIO (uint32_t adr, uint32_t val) { PSG_writeIO(psg, adr, val); }
    uint8_t readReg (uint32_t reg) { return PSG_readReg(psg, reg); }
    uint8_t readIO () { return PSG_readIO(psg); }
    int calc () { return PSG_calc(psg); }
    void setVolumeMode (int type) { PSG_setVolumeMode(psg, type); }
    uint32_t setMask (uint32_t mask) { return PSG_setMask(psg, mask); }
    uint32_t toggleMask (uint32_t mask) { return PSG_toggleMask(psg, mask); }
};

#endif
