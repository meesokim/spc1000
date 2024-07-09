#include "z80.h"

#define CPU_FREQ 4000000
class CPU {
        uint32_t cycles = 0;
        uint32_t prev = 0;
    public:
        z80 *r;
        CPU() { r = new z80(); }
        void initTick(uint32_t clk) { prev = clk; }
        uint32_t getCycles() { return cycles; }
        void init() { z80_init(r); r->interrupt_mode = 1;}
        void reset() { z80_reset(r); cycles = 0; }
        void set_pc(uint16_t pc) { r->pc = pc;}
        void set_sp(uint16_t sp) { r->sp = sp;}
        void step() { cycles += z80_step(r); }
        int  exec(int ms) { int steps = step_n((ms - prev) * CPU_FREQ / 1000); prev = ms; return steps; }
        int  step_n(unsigned ncycles) { return z80_step_n(r, ncycles); cycles += ncycles; }
        void debug() { z80_debug_output(r);}
        void assert_nmi() { z80_assert_nmi(r);}
        void pulse_nmi() { z80_pulse_nmi(r);}
        void clr_nmi() { z80_clr_nmi(r);}
        void assert_irq(uint8_t data) { z80_assert_irq(r, data);}
        void pulse_irq(uint8_t data) { z80_pulse_irq(r, data);}
        void clr_irq() { z80_clr_irq(r);}
        void set_read_write(uint8_t (*read)(void *, uint16_t), void (*write)(void *, uint16_t, uint8_t)) {
            // printf("read_byte:%x\n", read);
            r->read_byte = read;
            // printf("write_byte:%x\n", write);
            r->write_byte = write;
        }
        void set_in_out(uint8_t (*read)(z80 *, uint16_t), void (*write)(z80 *, uint16_t, uint8_t)) {
            r->port_in = read;
            r->port_out = write;
        }
};