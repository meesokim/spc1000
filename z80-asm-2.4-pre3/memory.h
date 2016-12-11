/* MEMORY INTERFACE */

#ifndef __MEMORY_H
#define __MEMORY_H

extern int init_banks(char *rom_path, char *bank_mapping_descr);
extern void reset_banks(void);
extern unsigned  bank_port_index(unsigned char id);
extern void switch_bank(_uchar val, unsigned map_no);

extern void set_default_byte(_uchar);
extern void clear_memory(void);

extern _uchar* io_address;
extern _uchar  memory_at(_ushort index);
extern _uchar  read_memo(_ushort index);
extern _uchar  read_opcode(_ushort index, bool set_m1);
extern void  write_memo(_ushort index, _uchar data);

/* used in asm.c by compile() respetively out()
   when called from z80-mon in 'put instruction into memory' */
extern _uchar write_to_memory(_ushort index, _uchar data);

extern unsigned  dma_write(_ushort offset, unsigned count, void *from);
extern unsigned  dma_read(_ushort offset, unsigned count, void *to);

#endif
