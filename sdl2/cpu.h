#include "z80.h"

class CPU {
    public:
        z80 *r;
        CPU() { r = new z80(); }
        void init() { z80_init(r);}
        void reset() { z80_reset(r);}
        void set_pc(uint16_t pc) { r->pc = pc;}
        void set_sp(uint16_t sp) { r->sp = sp;}
        void step() { z80_step(r); }
        void step_n(unsigned cycles) { z80_step_n(r, cycles);}
        void debug_output() { z80_debug_output(r);}
        void assert_nmi() { z80_assert_nmi(r);}
        void pulse_nmi() { z80_pulse_nmi(r);}
        void clr_nmi() { z80_clr_nmi(r);}
        void assert_irq(uint8_t data) { z80_assert_irq(r, data);}
        void pulse_irq(uint8_t data) { z80_pulse_irq(r, data);}
        void clr_irq() { z80_clr_irq(r);}
        void set_read_write(uint8_t (*read)(void *, uint16_t), void (*write)(void *, uint16_t, uint8_t)) {
            r->read_byte = read;
            r->write_byte = write;
        }
        void set_in_out(uint8_t (*read)(z80 *, uint16_t), void (*write)(z80 *, uint16_t, uint8_t)) {
            r->port_in = read;
            r->port_out = write;
        }
};