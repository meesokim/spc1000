/* EXECUTIVE DISASSEMBLING FUNCTIONS */
 
#ifndef __DISASM_H
#define __DISASM_H

extern int MODE;  /* used bits:  0,1, 2,3,4 */
extern void set_flag(_uchar flag);
extern _uchar is_flag(_uchar flag);

extern _uchar* f_nop (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_bit (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_res (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_set (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_ld (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_call (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_reti (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_retn (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_ret (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_inc (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_dec (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_rrca (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_rra (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_rla (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_rlca (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_add (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_adc (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_sub (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_sbc (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_djnz (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_jp (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_jr (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_daa (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_cpl (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_scf (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_ccf (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_halt (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_sdc (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_and (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_xor (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_or (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_cp (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_pop (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_push (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_rst (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_out (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_in (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_exx (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_di (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_ei (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_rrc (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_rlc (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_ex (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_rr (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_rl (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_sla (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_sll (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_srl (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_neg (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_sra (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_im (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_rrd (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_rld (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_ldd (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_lddr (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_ldi (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_ldir (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_cpd (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_cpdr (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_cpi (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_cpir (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_ind (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_indr (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_ini (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_inir (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_outd (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_otdr (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_outi (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_otir (_ushort a1,_uchar t1,_ushort a2,_uchar t2);
extern _uchar* f_nop2 (_ushort a1,_uchar t1,_ushort a2,_uchar t2);

#endif
