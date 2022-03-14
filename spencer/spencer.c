#include <spencer.h>

// needs memset
#include <string.h>

// needs printf
#include <stdio.h>

const uint8_t spc_ipl[0x40] = {
	0xCD, 0xEF, 0xBD, 0xE8, 0x00, 0xC6, 0x1D, 0xD0, 0xFC, 0x8F, 0xAA, 0xF4, 0x8F, 0xBB, 0xF5, 0x78,
	0xCC, 0xF4, 0xD0, 0xFB, 0x2F, 0x19, 0xEB, 0xF4, 0xD0, 0xFC, 0x7E, 0xF4, 0xD0, 0x0B, 0xE4, 0xF5,
	0xCB, 0xF4, 0xD7, 0x00, 0xFC, 0xD0, 0xF3, 0xAB, 0x01, 0x10, 0xEF, 0x7E, 0xF4, 0x10, 0xEB, 0xBA,
	0xF6, 0xDA, 0x00, 0xBA, 0xF4, 0xC4, 0xF4, 0xDD, 0x5D, 0xD0, 0xDB, 0x1F, 0x00, 0x00, 0xC0, 0xFF
};

void spc_init(spc_cpu* s, void* mem, uint8_t(*mmr)(spc_cpu*, uint8_t, uint8_t, uint8_t), void(*stop)(spc_cpu*)) {
	if(!mmr || !stop) return; // just dont initialize. bad data in, bad data out
	memset(s, 0, sizeof(spc_cpu));
	s->mmr = (uint8_t(*)(void*, uint8_t, uint8_t, uint8_t))mmr;
	s->mem = mem;
	s->stop = (void(*)(void*))stop;
	s->PC = 0xFFC0;
	s->wait = *spc_ipl;
	s->initialized = 0x59C700;
}

#define spc_r(s, p) spc_vr(s, p, 0)
#define spc_ir(s, p) spc_vr(s, p, S_I)

uint8_t spc_vr(spc_cpu* s, uint16_t p, uint8_t f) {
	if((p >= 0xF0 && p <= 0xF7) || (p >= 0xFA && p <= 0xFF)) return s->mmr(s, p - 0xF0, 0, S_R | f);
	if(p >= 0xFFC0 && s->mmr(s, 0xF1, 0, S_R | S_I) >> 7) return spc_ipl[p - 0xFFC0];
	return s->mem[p];
}

void spc_w(spc_cpu* s, uint16_t p, uint8_t d) {
	if((p >= 0xF0 && p <= 0xF7) || (p >= 0xFA && p <= 0xFF)) s->mmr(s, p - 0xF0, d, S_W);
	else s->mem[p] = d;
}

#define spc_getn(CPU) (CPU->P >> 7)
#define spc_getv(CPU) ((CPU->P >> 6) & 1)
#define spc_getp(CPU) ((CPU->P >> 5) & 1)
#define spc_getb(CPU) ((CPU->P >> 4) & 1)
#define spc_geth(CPU) ((CPU->P >> 3) & 1)
#define spc_geti(CPU) ((CPU->P >> 2) & 1)
#define spc_getz(CPU) ((CPU->P >> 1) & 1)
#define spc_getc(CPU) (CPU->P & 1)

#define spc_setn(CPU) CPU->P |= 0x80
#define spc_setv(CPU) CPU->P |= 0x40
#define spc_setp(CPU) CPU->P |= 0x20
#define spc_setb(CPU) CPU->P |= 0x10
#define spc_seth(CPU) CPU->P |= 0x08
#define spc_seti(CPU) CPU->P |= 0x04
#define spc_setz(CPU) CPU->P |= 0x02
#define spc_setc(CPU) CPU->P |= 0x01

#define spc_unsetn(CPU) CPU->P &= ~0x80
#define spc_unsetv(CPU) CPU->P &= ~0x40
#define spc_unsetp(CPU) CPU->P &= ~0x20
#define spc_unsetb(CPU) CPU->P &= ~0x10
#define spc_unseth(CPU) CPU->P &= ~0x08
#define spc_unseti(CPU) CPU->P &= ~0x04
#define spc_unsetz(CPU) CPU->P &= ~0x02
#define spc_unsetc(CPU) CPU->P &= ~0x01

#define spc_sbn(c, d) c->P = (c->P & 0x7F) | (d & 0x80)
#define spc_sbz(c, d) c->P = (c->P & 0xFD) | (!d << 1)

#define spc_sbnz(c, d) spc_sbn(c, d); spc_sbz(c, d)

#define spc_inc(c, d) d++; spc_sbnz(c, d)
#define spc_dec(c, d) d--; spc_sbnz(c, d)

#define spc_im8(C, O) (spc_r(C, C->PC + 1 + O))
#define spc_im16(C) spc_r16(s, C->PC+1)
#define spc_dp(C, O) ((((uint16_t)spc_getp(C)) << 8) | (uint8_t)(O))

#define spc_sp(C, O) (0x100 | (uint8_t)(O))

#define spc_push(C, O) spc_w(C, 0x100 + ((uint8_t)C->SP--), O)

#define spc_push16(C, O) spc_push(C, O >> 8); spc_push(C, O)

#define spc_pull(C) spc_r(C, C->SP++)

#define spc_or(C, A, B) A |= B; spc_sbnz(C, A)

uint16_t spc_pull16(spc_cpu* s) {
	uint16_t c = spc_pull(s);
	return c | ((uint16_t)spc_pull(s) << 8);
}

#define spc_setbf(C, S) C |= (1 << S)
#define spc_unsetbf(C, S) C &= ~(1 << S)

#define spc_branch(C, O) s->PC += (int8_t)spc_im8(C, O)

typedef struct {
	uint16_t adr;
	uint8_t bit;
} spc_mb;

#define spc_splitmb(O) ((spc_mb){O & 0x1FFF, O >> 13})

#define spc_getya(C) = ((uint16_t)C->A | (((uint16_t)C->Y) << 8))

// cool guy macros
#define spc_getbf(C, S) ((C >> S) & 1) 
#define spc_trans(c, a, b) a = b; spc_sbnz(c, a)

uint16_t spc_r16(spc_cpu* s, uint16_t p) {
	uint16_t d = spc_r(s, p++);
	return (d << 8) | spc_r(s, p);
}

#define SPC_VT 0
int8_t spc_cycles[256] = { // -1: length returned by spc_eval
//	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
	2,	8,	4,	SPC_VT,	3,	4,	3,	6,	2,	6,	5,	4,	5,	4,	6,	8,	// 0
	SPC_VT,	8,	4,	SPC_VT,	4,	5,	5,	6,	5,	5,	6,	5,	2,	2,	4,	6,	// 1
	2,	8,	4,	SPC_VT,	3,	4,	3,	6,	2,	6,	5,	6,	5,	4,	SPC_VT,	4,	// 2
	SPC_VT,	8,	4,	SPC_VT,	4,	5,	5,	6,	5,	5,	6,	5,	2,	2,	3,	8,	// 3
	2,	8,	4,	SPC_VT,	3,	4,	3,	6,	2,	6,	4,	4,	5,	4,	6,	6,	// 4
	SPC_VT,	8,	4,	SPC_VT,	4,	5,	5,	6,	5,	5,	4,	5,	2,	2,	4,	3,	// 5
	2,	8,	4,	SPC_VT,	3,	4,	3,	6,	2,	6,	4,	4,	5,	4,	SPC_VT,	5,	// 6
	SPC_VT,	8,	4,	SPC_VT,	4,	5,	5,	6,	5,	5,	5,	5,	2,	2,	3,	6,	// 7
	2,	8,	4,	SPC_VT,	3,	4,	3,	6,	2,	6,	5,	4,	5,	2,	4,	5,	// 8
	SPC_VT,	8,	4,	SPC_VT,	4,	5,	5,	6,	5,	5,	5,	5,	2,	2,	12,	5,	// 9
	3,	8,	4,	SPC_VT,	3,	4,	3,	6,	2,	6,	4,	4,	5,	2,	4,	4,	// a
	SPC_VT,	8,	4,	SPC_VT,	4,	5,	5,	6,	5,	5,	5,	5,	2,	2,	3,	4,	// b
	3,	8,	4,	SPC_VT,	4,	5,	4,	7,	2,	5,	6,	4,	5,	2,	4,	9,	// c
	SPC_VT,	8,	4,	SPC_VT,	5,	6,	6,	7,	4,	5,	5,	5,	2,	2,	SPC_VT,	3,	// d
	2,	8,	4,	SPC_VT,	3,	4,	3,	6,	2,	4,	5,	3,	4,	3,	4,	SPC_VT,	// e
	SPC_VT,	8,	4,	SPC_VT,	4,	5,	5,	6,	3,	4,	5,	4,	2,	2,	SPC_VT,	SPC_VT	// f
};

uint8_t spc_len[256] = {
//	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
	1,	1,	2,	3,	2,	3,	1,	2,	2,	3,	3,	2,	3,	1,	3,	1,	// 0
	2,	1,	2,	3,	2,	3,	3,	2,	3,	1,	2,	2,	1,	1,	3,	3,	// 1
	1,	1,	2,	3,	2,	3,	1,	2,	2,	3,	3,	2,	3,	1,	3,	2,	// 2
	2,	1,	2,	3,	2,	3,	3,	2,	3,	1,	2,	2,	1,	1,	2,	3,	// 3
	1,	1,	2,	3,	2,	3,	1,	2,	2,	3,	3,	2,	3,	1,	3,	2,	// 4
	2,	1,	2,	3,	2,	3,	3,	2,	3,	1,	2,	2,	1,	1,	3,	3,	// 5
	1,	1,	2,	3,	2,	3,	1,	2,	2,	3,	3,	2,	3,	1,	3,	1,	// 6
	2,	1,	2,	3,	2,	3,	3,	2,	3,	1,	2,	2,	1,	1,	2,	1,	// 7
	1,	1,	2,	3,	2,	3,	1,	2,	2,	3,	3,	2,	3,	2,	1,	3,	// 8
	2,	1,	2,	3,	2,	3,	3,	2,	3,	1,	2,	2,	1,	1,	1,	1,	// 9
	1,	1,	2,	3,	2,	3,	1,	2,	2,	3,	3,	2,	3,	2,	1,	1,	// a
	2,	1,	2,	3,	2,	3,	3,	2,	3,	1,	2,	2,	1,	1,	1,	1,	// b
	1,	1,	2,	3,	2,	3,	1,	2,	2,	3,	3,	2,	3,	2,	1,	1,	// c
	2,	1,	2,	3,	2,	3,	3,	2,	2,	2,	2,	2,	1,	1,	3,	1,	// d
	1,	1,	2,	3,	2,	3,	1,	2,	2,	3,	3,	2,	3,	1,	1,	1,	// e
	2,	1,	2,	3,	2,	3,	3,	2,	2,	2,	3,	2,	1,	1,	2,	1	// f
};

void spc_eval(spc_cpu* s, int* d, uint8_t opcode) {
	uint8_t ri;
	uint16_t adr;
	spc_mb mb;
	if((opcode & 0xF) == 1) { // tcall (0x*1)
		spc_push16(s, s->PC);
		// should be xFFDE, but spc_len adds 3 to
		// PC anyway so i subtracted 3 from here.
		// if you copy the code keep that in mind
		s->PC = spc_r16(s, 0xFFDB - (opcode >> 3));
	} else if((opcode & 0x0F) == 2) { // set1 (evens x*2), clr1(odds x*2)
		uint8_t b = spc_r(s, spc_dp(s, spc_im8(s, 0)));
		if(opcode & 0x10) spc_unsetbf(b, opcode >> 5);
		else spc_setbf(b, opcode >> 5);
		spc_w(s, spc_dp(s, spc_im8(s, 0)), b);
	} else if((opcode << 3) == 0x80) {
		// - GUIDE -  reg !if opcode id
		// bpl (x10), 00  0   10000
		// bmi (x30), 00  1   10000
		//
		// bvc (x50), 01  0   10000
		// bvs (x70), 01  1   10000
		//
		// bcc (x90), 10  0   10000
		// bcs (xB0), 10  1   10000
		//
		// bne (xD0), 11  0   10000
		// beq (xF0), 11  1   10000
		uint8_t conflags[4] = {spc_getn(s), spc_getv(s), spc_getc(s), spc_getz(s)};
		if(conflags[opcode >> 6] & spc_getbf(opcode, 5)) spc_branch(s, 0);
	} else if((opcode & 0x1F) == 0xA && opcode <= 0x6A) {
		mb = spc_splitmb(spc_im16(s));
		ri = spc_getbf(spc_r(s, mb.adr), mb.bit) ^ spc_getbf(opcode, 6);
		if(opcode & 0x40) s->P |= ri;
		else s->P &= ri;
	} else if((opcode & 0x0F) == 3) {
		uint8_t set = spc_getbf(spc_r(s, spc_im8(s, 0)), opcode >> 5);
		if((opcode & 0x10) && !set) goto br;
		else if(!(opcode & 0x10) && set) goto br;
		else goto nobr;

br:	spc_branch(s, 1);
	*d = 2;
nobr:

	} else if((opcode & 0x0F) == 0x0D && !(opcode & 0x10) && opcode <= 0x6D) {
		uint8_t regs[4] = {s->P, s->A, s->X, s->Y};
		spc_push(s, regs[opcode >> 5]);
	} else if((opcode & 0x0F) == 0x0E && opcode >= 0x8E && !(opcode & 0x10)) {
		uint8_t* regs[4] = {&s->P, &s->A, &s->X, &s->Y};
		*regs[opcode >> 5] = spc_pull(s);
	} else switch(opcode) {
		case 0x00:
			break;
		case 0x04:
			spc_or(s, s->A, spc_r(s, spc_dp(s, spc_im8(s, 0))));
			break;
		case 0x05:
			spc_or(s, s->A, spc_r(s, spc_im16(s)));
			break;
		case 0x0F:
			spc_push16(s, s->PC);
			spc_push(s, s->P);
			s->PC = 0xFFDD;
			s->P |= 0x10;
			s->P &= 0xFB;
			break;
		case 0x1D:
			spc_dec(s, s->X);
			break;
		case 0x20:
			spc_unsetp(s);
			break;
		case 0x2F:
			spc_branch(s, 0);
			break;
		case 0x3D:
			spc_inc(s, s->X);
			break;
		case 0x3F:
			spc_push16(s, s->PC);
			s->PC = spc_im16(s) - 3; // subtract 3 for opcode length
			break;
		case 0x40:
			spc_setp(s);
			break;
		case 0x4F:
			spc_push16(s, s->PC);
			s->PC = (0xFF00 | spc_im8(s, 0)) - 2; // subtract 2 for opcode length
			break;
		case 0x5D:
			spc_trans(s, s->X, s->A);
			break;
		case 0x60:
			spc_unsetc(s);
			break;
		case 0x6F:
			s->PC = spc_pull16(s) - 1;
			break;
		case 0x7D:
			spc_trans(s, s->A, s->X);
			break;
		case 0x7F:
			s->P = spc_pull(s);
			s->PC = spc_pull16(s) - 1;
			break;
		case 0x80:
			spc_setc(s);
			break;
		case 0x8A:
			adr = spc_im16(s);
			spc_mb mb = spc_splitmb(adr);
			s->P ^= spc_getbf(spc_r(s, mb.adr), mb.bit);
			break;
		case 0x8B:
			adr = spc_dp(s, spc_im8(s, 0));
			ri = spc_r(s, adr) - 1;
			spc_w(s, adr, ri);
			spc_sbnz(s, ri);
			break;
		case 0x8C:
			adr = spc_im16(s);
			ri = spc_r(s, adr) - 1;
			spc_w(s, adr, ri);
			spc_sbnz(s, ri);
			break;
		case 0x8D:
			spc_trans(s, s->Y, spc_im8(s, 0));
			break;
		case 0x9B:
			adr = spc_dp(s, spc_im8(s, 0) + s->X);
			ri = spc_r(s, adr) - 1;
			spc_w(s, adr, ri);
			spc_sbnz(s, ri);
			break;
		case 0x9C:
			spc_dec(s, s->A);
			break;
		case 0x9D:
			spc_trans(s, s->X, s->SP);
			break;
		case 0x9F:
			s->A = (s->A >> 4) | (s->A << 4);
			spc_sbn(s, s->A);
			spc_sbz(s, s->A);
			break;
		case 0xA0:
			spc_seti(s);
			break;
		case 0xAA:
			adr = spc_im16(s);
			mb = spc_splitmb(adr);
			s->P = (s->P & 0xFE) | (spc_getbf(spc_r(s, mb.adr), mb.bit));
			break;
		case 0xBC:
			spc_inc(s, s->A);
			break;
		case 0xBD:
			spc_trans(s, s->SP, s->X);
			break;
		case 0xC0:
			spc_unseti(s);
			break;
		case 0xCA:
			adr = spc_im16(s);
			mb = spc_splitmb(adr);
			spc_w(s, mb.adr, (spc_r(s, mb.adr) & ~(1 << mb.bit)) | (spc_getc(s) << mb.bit));
			break;
		case 0xCD:
			spc_trans(s, s->X, spc_im8(s, 0));
			break;
		case 0xDC:
			spc_dec(s, s->Y);
			break;
		case 0xDD:
			spc_trans(s, s->A, s->Y);
			break;
		case 0xE0:
			spc_unsetv(s);
			spc_unseth(s);
			break;
		case 0xE8:
			spc_trans(s, s->A, spc_im8(s, 0));
			break;
		case 0xEA:
			adr = spc_im16(s);
			mb = spc_splitmb(adr);
			spc_w(s, mb.adr, spc_r(s, mb.adr) ^ (1 << mb.bit));
			break;
		case 0xED:
			s->P ^= 1;
			break;
		case 0xEF:
			s->stp = 0xEF;
			s->stop(s);
			break;
		case 0xFC:
			spc_inc(s, s->Y);
			break;
		case 0xFD:
			spc_trans(s, s->Y, s->A);
			break;
		case 0xFF:
			s->stop(s);
			s->stp = 0xFF;
			break;
		default:
			printf("unimplemented opcode: 0x%02X\n", spc_r(s, s->PC));
			*d = -1;
			break;
	}
}

/* opcodes that need the Variable Timingness (they are branches)
	0x2E
	0x6E
	0xDE
	0xFE
*/

int spc_loop(spc_cpu* s) {
	if(s->initialized != 0x59C700) return !printf("SPENCER UNINITIALIZED\n") - 1;
	if(s->stp) return 0;
	int d = 0;
	if(s->wait == 0) {
		printf("-- EVAL --\n");
		if(!s->con) {
			uint8_t opcode = spc_r(s, s->PC);
			spc_eval(s, &d, opcode);
			if(d < 0) return d;
			if(d <= 2) {
				s->wait += d;
				s->con = 1;
			}
			s->PC += spc_len[opcode];
			s->wait = spc_cycles[opcode];
		} else s->con = 0;
	} else s->wait--;
	return d;
}
