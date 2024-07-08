#ifndef AY8910_H
#define AY8910_H

extern "C" {
#include "emu2149.h"
};

class AY8910
{
    PSG *psg;
public:
    uint8_t reg;
    AY8910(uint32_t clk = 4000000, uint32_t rate = 44100) 
    {
        psg = PSG_new(clk, rate);
    }
    ~AY8910() { PSG_delete (psg); }
    void set_quality (uint32_t q) { PSG_set_quality(psg, q); }
    void set_rate (uint32_t r) { PSG_set_rate(psg, r); }
    void reset () { PSG_reset(psg);}
    void latch (uint8_t val) { reg == val; }
    void write (uint8_t val) { PSG_writeReg(psg, reg, val); }
    uint8_t read () { return readReg(reg); }
    void writeReg (uint32_t reg, uint32_t val) { PSG_writeReg(psg, reg, val); }
    void writeIO (uint32_t adr, uint32_t val) { PSG_writeIO(psg, adr, val); }
    uint8_t readReg (uint32_t reg) { return PSG_readReg(psg, reg); }
    uint8_t readIO () { return PSG_readIO(psg); }
    int16_t calc () { return PSG_calc(psg); }
    void setVolumeMode (int type) { PSG_setVolumeMode(psg, type); }
    uint32_t setMask (uint32_t mask) { return PSG_setMask(psg, mask); }
    uint32_t toggleMask (uint32_t mask) { return PSG_toggleMask(psg, mask); }
};

#endif
