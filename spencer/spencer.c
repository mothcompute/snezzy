#include <spencer.h>

// needs memset
#include <string.h>

// needs printf
#include <stdio.h>

void spc_init(spc_cpu* s, void* mem, uint8_t(*mmr)(spc_cpu*, uint8_t, uint8_t, uint8_t), void(*stop)(spc_cpu*)) {
	if(!mmr || !stop) return; // just dont initialize. bad data in, bad data out
	memset(s, 0, sizeof(spc_cpu));
	s->mmr = (uint8_t(*)(void*, uint8_t, uint8_t, uint8_t))mmr;
	s->mem = mem;
	s->stop = (void(*)(void*))stop;
	s->initialized = 0x59C700;
}

const uint8_t spc_ipl[0x40] = {
	0xCD, 0xEF, 0xBD, 0xE8, 0x00, 0xC6, 0x1D, 0xD0, 0xFC, 0x8F, 0xAA, 0xF4, 0x8F, 0xBB, 0xF5, 0x78,
	0xCC, 0xF4, 0xD0, 0xFB, 0x2F, 0x19, 0xEB, 0xF4, 0xD0, 0xFC, 0x7E, 0xF4, 0xD0, 0x0B, 0xE4, 0xF5,
	0xCB, 0xF4, 0xD7, 0x00, 0xFC, 0xD0, 0xF3, 0xAB, 0x01, 0x10, 0xEF, 0x7E, 0xF4, 0x10, 0xEB, 0xBA,
	0xF6, 0xDA, 0x00, 0xBA, 0xF4, 0xC4, 0xF4, 0xDD, 0x5D, 0xD0, 0xDB, 0x1F, 0x00, 0x00, 0xC0, 0xFF
};

uint8_t spc_r(spc_cpu* s, uint16_t p) {
	if((p >= 0xF0 && p <= 0xF7) || (p >= 0xFA && p <= 0xFF)) return s->mmr(s, p - 0xF0, 0, S_R);
	if(p >= 0xFFC0 && s->mmr(s, 0xF1, 0, S_R|S_I) >> 7) return spc_ipl[p - 0xFFC0];
	return s->mem[p];
}

uint8_t spc_intr(spc_cpu* s, uint16_t p) {
	if((p >= 0xF0 && p <= 0xF7) || (p >= 0xFA && p <= 0xFF)) return s->mmr(s, p - 0xF0, 0, S_R|S_I);
	return spc_r(s, p);
}

void spc_w(spc_cpu* s, uint16_t p, uint8_t d) {
	if((p >= 0xF0 && p <= 0xF7) || (p >= 0xFA && p <= 0xFF)) {
		s->mmr(s, p - 0xF0, d, S_W);
		return;
	}
	s->mem[p] = d;
}

// this is bad
#define spc_im8(C, O) (spc_r(C, C->PC + 1 + O))

// this is not okay
#define spc_im16(C) ((((uint16_t)spc_im8(C, 1)) << 8) | spc_im8(C, 0))

// with code like this all these opcodes are illegal opcodes
#define spc_dp(C, O) ((((uint16_t)spc_getp(C)) << 8) | (uint8_t)O)

#define spc_sp(C, O) (0x100 | (uint8_t)O)

#define spc_push(C, O) spc_w(C, 0x100 + ((uint8_t)C->SP--), O)

#define spc_pull(C) spc_r(C, C->SP++)

#define spc_setbf(C, S) C |= (1 << S)
#define spc_unsetbf(C, S) C &= ~(1 << S)
#define spc_issetbf(C, S) ((C >> S) & 1)

#define SPC_VT -1
int spc_cycles[256] = { // -1: length returned by spc_eval
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

void spc_eval(spc_cpu* s, int* d) {
	switch(spc_r(s, s->PC)) {
		case 0x00:
			break;
		case 0x20:
			spc_unsetp(s);
			break;
		case 0x3D:
			spc_inc(s, s->X);
			break;
		case 0x40:
			spc_setp(s);
			break;
		case 0x5D:
			spc_trans(s, s->X, s->A);
			break;
		case 0x60:
			spc_unsetc(s);
			break;
		case 0x80:
			spc_setc(s);
			break;
		case 0xA0:
			spc_seti(s);
			break;
		case 0xBC:
			spc_inc(s, s->A);
			break;
		case 0xC0:
			spc_unseti(s);
			break;
		case 0xE0:
			spc_unsetv(s);
			spc_unseth(s);
			break;
		case 0xED:
			s->P ^= 1;
			break;
		case 0xEF:
			s->stop(s);
			break;
		case 0xFC:
			spc_inc(s, s->Y);
			break;
		case 0xFF:
			s->stop(s);
			break;
		default:
			printf("unimplemented opcode: 0x%02X\n", spc_r(s, s->PC));
			*d = 1;
			break;
	}
}

int spc_cyc(spc_cpu* s) {
	switch(spc_r(s, s->PC)) {
		case 0x03:
			break;
		case 0x10:
			break;
		case 0x13:
			break;
		case 0x23:
			break;
		case 0x2E:
			break;
		case 0x30:
			break;
		case 0x33:
			break;
		case 0x43:
			break;
		case 0x50:
			break;
		case 0x53:
			break;
		case 0x63:
			break;
		case 0x6E:
			break;
		case 0x70:
			break;
		case 0x73:
			break;
		case 0x83:
			break;
		case 0x90:
			break;
		case 0x93:
			break;
		case 0xA3:
			break;
		case 0xB0:
			break;
		case 0xB3:
			break;
		case 0xC3:
			break;
		case 0xD0:
			break;
		case 0xD3:
			break;
		case 0xDE:
			break;
		case 0xE3:
			break;
		case 0xEF:
			s->stp = 0xEF;
			s->stop(s);
			return 0;
			break;
		case 0xF0:
			break;
		case 0xF3:
			break;
		case 0xFE:
			break;
		case 0xFF:
			s->stp = 0xFF;
			s->stop(s);
			return 0;
			break;
		default:
			break;
	}
	return spc_cycles[spc_r(s, s->PC)] - 1;
}

int spc_loop(spc_cpu* s) {
	if(s->initialized != 0x59C700) return !printf("SPENCER UNINITIALIZED\n") - 1;
	if(!s->use) s->use = !!(s->wait = spc_cyc(s));
	if(s->stop) return 0;
	int err = 0;
	if(s->wait == 0) {
		printf("-- EVAL --\n");
		spc_eval(s, &err);
		s->PC += spc_len[spc_r(s, s->PC)];
		s->wait = spc_cyc(s);
	} else s->wait--;
	return err;
}
