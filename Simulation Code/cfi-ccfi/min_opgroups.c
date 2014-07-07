/*care:    (pc, ci, op, opmove, oparith2s1d, mr)
rescare: (pc, oparith1s1d, mw)
12m  p3m
*/

/*  opgroups  */
#define move 0x01
#define move_care 0x38
#define move_rescare 0x04

#define arith2s1d 0x01
#define arith2s1d_care 0x38
#define arith2s1d_rescare 0x04

#define arith1s1d 0x01
#define arith1s1d_care 0x38
#define arith1s1d_rescare 0x04

#define arith1d 0x01
#define arith1d_care 0x38
#define arith1d_rescare 0x04

#define wload 0x01
#define wload_care 0x38
#define wload_rescare 0x04

#define bload 0x01
#define bload_care 0x38
#define bload_rescare 0x04

#define wstore 0x01
#define wstore_care 0x38
#define wstore_rescare 0x45

#define bstore 0x01
#define bstore_care 0x38
#define bstore_rescare 0x04

#define cbranch 0x01
#define cbranch_care 0x38
#define cbranch_rescare 0x04

#define ubranch 0x01
#define ubranch_care 0x38
#define ubranch_rescare 0x04

#define ijump 0x02
#define ijump_care 0x38
#define ijump_rescare 0x04

#define dcall 0x01
#define dcall_care 0x38
#define dcall_rescare 0x04

#define icall 0x02
#define icall_care 0x38
#define icall_rescare 0x04

#define ret 0x02
#define ret_care 0x38
#define ret_rescare 0x04

#define other 0x0f
#define other_care 0x38
#define other_rescare 0x00

/******************************/

#include <stdio.h>
#include <string.h>

struct isa {
    char *str;
    unsigned int opgrp;
    unsigned int care;
    unsigned int rescare;
};

struct isa dict[] = {	/* 332 */
    {"addl", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"addlimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"addlv", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"addlvimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"addq", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"addqimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"addqv", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"addqvimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"adds", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"adds/su", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"adds/sud", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"adds/sum", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"addt", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"addt/sud", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"addt/sum", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"amask", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"amaskimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"and", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"andimm", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"arm", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"beq", 			cbranch,	cbranch_care,	cbranch_rescare},
    {"bge", 			cbranch,	cbranch_care,	cbranch_rescare},
    {"bgt", 			cbranch,	cbranch_care,	cbranch_rescare},
    {"bic", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"bicimm", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"bis", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"bisimm", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"blbc", 			cbranch,	cbranch_care,	cbranch_rescare},
    {"blbs", 			cbranch,	cbranch_care,	cbranch_rescare},
    {"ble", 			cbranch,	cbranch_care,	cbranch_rescare},
    {"blt", 			cbranch,	cbranch_care,	cbranch_rescare},
    {"bne", 			cbranch,	cbranch_care,	cbranch_rescare},
    {"br", 			ubranch,	ubranch_care,	ubranch_rescare},
    {"brandlink", 		ubranch,	ubranch_care,	ubranch_rescare},
    {"bsr", 			dcall,	dcall_care,	dcall_rescare},
    {"bsrandlink", 		dcall,	dcall_care,	dcall_rescare},
    {"call_pal", 		other,	other_care,	other_rescare},
    {"callsys", 		other,	other_care,	other_rescare},
    {"cmoveq", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmoveqimm", 		move,	move_care,	move_rescare},
    {"cmovge", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmovgeimm", 		move,	move_care,	move_rescare},
    {"cmovgt", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmovgtimm", 		move,	move_care,	move_rescare},
    {"cmovlbc", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmovlbcimm", 		move,	move_care,	move_rescare},
    {"cmovlbs", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmovlbsimm", 		move,	move_care,	move_rescare},
    {"cmovle", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmovleimm", 		move,	move_care,	move_rescare},
    {"cmovlt", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmovltimm", 		move,	move_care,	move_rescare},
    {"cmovne", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmovneimm", 		move,	move_care,	move_rescare},
    {"cmpbge", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmpbgeimm", 		move,	move_care,	move_rescare},
    {"cmpeq", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmpeqimm", 		move,	move_care,	move_rescare},
    {"cmple", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmpleimm", 		move,	move_care,	move_rescare},
    {"cmplt", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmpltimm", 		move,	move_care,	move_rescare},
    {"cmpteq", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmptle", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmptlt", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmptun", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmpule", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmpuleimm", 		move,	move_care,	move_rescare},
    {"cmpult", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"cmpultimm", 		move,	move_care,	move_rescare},
    {"cpys", 			other,	other_care,	other_rescare},
    {"cpyse", 			other,	other_care,	other_rescare},
    {"cpysn", 			other,	other_care,	other_rescare},
    {"ctlz", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"ctpop", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cttz", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cvtlq", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cvtql", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cvtqlv", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cvtqs", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cvtqs/d",			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cvtqt", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cvtqt/d",			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cvtst", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cvttq", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cvttq/c", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cvttq/suc", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cvttqchopped", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cvttqminusinfinity",	arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"cvtts", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"deprecated_exit",		other,	other_care,	other_rescare},
    {"deprecated_ivlb",		other,	other_care,	other_rescare},
    {"deprecated_ivle",		other,	other_care,	other_rescare},
    {"divs", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"divs/sud", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"divt", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"divt/c", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"divt/su", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"divt/sud", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"dumpresetstats", 		other,	other_care,	other_rescare},
    {"dumpstats", 		other,	other_care,	other_rescare},
    {"eqv", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"eqvimm", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"excb", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"extbl", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"extblimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"extlh", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"extlhimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"extll", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"extllimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"extqh", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"extqhimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"extql", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"extqlimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"extwh", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"extwhimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"extwl", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"extwlimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"fbeq", 			cbranch,	cbranch_care,	cbranch_rescare},
    {"fbge", 			cbranch,	cbranch_care,	cbranch_rescare},
    {"fbgt", 			cbranch,	cbranch_care,	cbranch_rescare},
    {"fble", 			cbranch,	cbranch_care,	cbranch_rescare},
    {"fblt", 			cbranch,	cbranch_care,	cbranch_rescare},
    {"fbne", 			cbranch,	cbranch_care,	cbranch_rescare},
    {"fcmoveq", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"fcmovge", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"fcmovgt", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"fcmovle", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"fcmovlt", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"fcmovne", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"ftois", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"ftoit", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"halt", 			other,	other_care,	other_rescare},
    {"hw_ldl", 			other,	other_care,	other_rescare},
    {"hw_ldq", 			other,	other_care,	other_rescare},
    {"hw_mfpr", 		other,	other_care,	other_rescare},
    {"hw_mtpr", 		other,	other_care,	other_rescare},
    {"hw_rei", 			other,	other_care,	other_rescare},
    {"hw_stl", 			other,	other_care,	other_rescare},
    {"hw_stq", 			other,	other_care,	other_rescare},
    {"implver", 		other,	other_care,	other_rescare},
    {"initparam", 		other,	other_care,	other_rescare},
    {"insbl", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"insblimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"inslh", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"inslhimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"insll", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"insllimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"insqh", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"insqhimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"insql", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"insqlimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"inswh", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"inswhimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"inswl", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"inswlimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"itofs", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"itoft", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"jmp", 			ijump,	ijump_care,	ijump_rescare},
    {"jmpandlink", 		ijump,	ijump_care,	ijump_rescare},
    {"jsr", 			icall,	icall_care,	icall_rescare},
    {"jsr_coroutine", 		icall,	icall_care,	icall_rescare},
    {"jsr_coroutineandlink",	icall,	icall_care,	icall_rescare},
    {"jsrandlink", 		icall,	icall_care,	icall_rescare},
    {"lda", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"ldah", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"ldbu", 			bload,	bload_care,	bload_rescare},
    {"ldl", 			bload,	bload_care,	bload_rescare},
    {"ldl_l", 			bload,	bload_care,	bload_rescare},
    {"ldlprefetch", 		bload,	bload_care,	bload_rescare},
    {"ldq", 			wload,	wload_care,	wload_rescare},
    {"ldq_l", 			wload,	wload_care,	wload_rescare},
    {"ldq_u", 			wload,	wload_care,	wload_rescare},
    {"ldqprefetch", 		wload,	wload_care,	wload_rescare},
    {"lds", 			bload,	bload_care,	bload_rescare},
    {"ldsprefetch", 		bload,	bload_care,	bload_rescare},
    {"ldt", 			wload,	wload_care,	wload_rescare},
    {"ldwu", 			wload,	wload_care,	wload_rescare},
    {"loadsymbol", 		wload,	wload_care,	wload_rescare},
    {"m5a_aq", 			other,	other_care,	other_rescare},
    {"m5a_begin", 		other,	other_care,	other_rescare},
    {"m5a_bsm", 		other,	other_care,	other_rescare},
    {"m5a_dq", 			other,	other_care,	other_rescare},
    {"m5a_end", 		other,	other_care,	other_rescare},
    {"m5a_esm", 		other,	other_care,	other_rescare},
    {"m5a_getid", 		other,	other_care,	other_rescare},
    {"m5a_identify", 		other,	other_care,	other_rescare},
    {"m5a_l", 			other,	other_care,	other_rescare},
    {"m5a_old", 		other,	other_care,	other_rescare},
    {"m5a_pq", 			other,	other_care,	other_rescare},
    {"m5a_q", 			other,	other_care,	other_rescare},
    {"m5a_rq", 			other,	other_care,	other_rescare},
    {"m5a_scl", 		other,	other_care,	other_rescare},
    {"m5a_sq", 			other,	other_care,	other_rescare},
    {"m5a_we", 			other,	other_care,	other_rescare},
    {"m5a_wf", 			other,	other_care,	other_rescare},
    {"m5addsymbol", 		other,	other_care,	other_rescare},
    {"m5break", 		other,	other_care,	other_rescare},
    {"m5checkpoint", 		other,	other_care,	other_rescare},
    {"m5exit", 			other,	other_care,	other_rescare},
    {"m5panic", 		other,	other_care,	other_rescare},
    {"m5readfile", 		other,	other_care,	other_rescare},
    {"m5reserved2", 		other,	other_care,	other_rescare},
    {"m5reserved3", 		other,	other_care,	other_rescare},
    {"m5reserved4", 		other,	other_care,	other_rescare},
    {"m5reserved5", 		other,	other_care,	other_rescare},
    {"m5switchcpu", 		other,	other_care,	other_rescare},
    {"maxsb8", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"maxsw4", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"maxub8", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"maxuw4", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mb", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mf_fpcr", 		other,	other_care,	other_rescare},
    {"minsb8", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"minsw4", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"minub8", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"minuw4", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mskbl", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mskblimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"msklh", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"msklhimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"mskll", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mskllimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"mskqh", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mskqhimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"mskql", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mskqlimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"mskwh", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mskwhimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"mskwl", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mskwlimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"mt_fpcr", 		other,	other_care,	other_rescare},
    {"mull", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mullimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"mullv", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mullvimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"mulq", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mulqimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"mulqv", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mulqvimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"muls", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"muls/su", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"muls/sud", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mult", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mult/su", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"mult/sud", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"nop", 			other,	other_care,	other_rescare},
    {"ornot", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"ornotimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"perr", 			other,	other_care,	other_rescare},
    {"pklb", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"pkwb", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"quiesce", 		other,	other_care,	other_rescare},
    {"quiescecycles", 		other,	other_care,	other_rescare},
    {"quiescens", 		other,	other_care,	other_rescare},
    {"quiescetime", 		other,	other_care,	other_rescare},
    {"rc", 			other,	other_care,	other_rescare},
    {"rduniq", 			other,	other_care,	other_rescare},
    {"resetstats", 		other,	other_care,	other_rescare},
    {"ret", 			ret,	ret_care,	ret_rescare},
    {"retandlink", 		ret,	ret_care,	ret_rescare},
    {"return", 			ret,	ret_care,	ret_rescare},
    {"rpcc", 			other,	other_care,	other_rescare},
    {"rpns", 			other,	other_care,	other_rescare},
    {"rs", 			other,	other_care,	other_rescare},
    {"s4addl", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"s4addlimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"s4addq", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"s4addqimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"s4subl", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"s4sublimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"s4subq", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"s4subqimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"s8addl", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"s8addlimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"s8addq", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"s8addqimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"s8subl", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"s8sublimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"s8subq", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"s8subqimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"sextb", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"sextbimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"sextw", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"sextwimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"sll", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"sllimm", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"sqrts", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"sqrtt", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"sra", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"sraimm", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"srl", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"srlimm", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"stb", 			bstore,	bstore_care,	bstore_rescare},
    {"stl", 			bstore,	bstore_care,	bstore_rescare},
    {"stl_c", 			bstore,	bstore_care,	bstore_rescare},
    {"stq", 			wstore,	wstore_care,	wstore_rescare},
    {"stq_c", 			wstore,	wstore_care,	wstore_rescare},
    {"stq_u", 			wstore,	wstore_care,	wstore_rescare},
    {"sts", 			bstore,	bstore_care,	bstore_rescare},
    {"stt", 			wstore,	wstore_care,	wstore_rescare},
    {"stw", 			wstore,	wstore_care,	wstore_rescare},
    {"subl", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"sublimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"sublv", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"sublvimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"subq", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"subqimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"subqv", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"subqvimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"subs", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"subs/su", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"subs/sud", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"subs/sum", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"subt", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"subt/sud", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"subt/sum", 		arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"trapb", 			other,	other_care,	other_rescare},
    {"umulh", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"umulhimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"unpkbl", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"unpkbw", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"wakecpu", 		other,	other_care,	other_rescare},
    {"wh64", 			other,	other_care,	other_rescare},
    {"wmb", 			other,	other_care,	other_rescare},
    {"wruniq", 			other,	other_care,	other_rescare},
    {"xor", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"xorimm", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"zap", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"zapimm", 			arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {"zapnot", 			arith2s1d,	arith2s1d_care,	arith2s1d_rescare},
    {"zapnotimm", 		arith1s1d,	arith1s1d_care,	arith1s1d_rescare},
    {NULL , 		 	0xff,	other_care,	other_rescare}
};

static int dict_size = -1;
void init_dict_size()
{
	dict_size = 0;
	while(dict[dict_size].opgrp > 0)
	{
		dict_size++;
	}
}

int isa_cmp(const void *key, const void * elem)
{
	//assert(dict_size>0);
	return strcmp((char *)key, ((struct isa *)elem)->str);
}

int opcodeToInt(char *key, unsigned int* opgrp, unsigned int* care, unsigned int* rescare)
{
	struct isa *match;

	assert(dict_size >= 0);

	match = bsearch(key,
			dict,
			dict_size,
			sizeof(struct isa),
			&isa_cmp);


	if(match)
	{
		*opgrp   = match->opgrp;
		*care    = match->care;
		*rescare = match->rescare;
		return 1;//match->n;
	}


	*opgrp   = -1;
	*care    = -1;
	*rescare = -1;

	return 0;
}
