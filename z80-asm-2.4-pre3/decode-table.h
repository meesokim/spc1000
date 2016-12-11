/* INSTRUCTION TABLES FOR DISASSEMBLING */

#include "z80-global"

extern struct instruction_type instr_normal[256];  /* no prefix instructions */
extern struct instruction_type instr_cb[256];  /* cb prefix instructions */
extern struct instruction_type instr_ed[256];  /* ed prefix instructions */
