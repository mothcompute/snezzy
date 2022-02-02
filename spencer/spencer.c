#include <spencer.h>

// needs memset
#include <string.h>

// needs printf
#include <stdio.h>

void spc_init(spc_cpu* s, void* mem, uint8_t(*mmr)(uint8_t, uint8_t, uint8_t)) {
	memset(s, 0, sizeof(spc_cpu));
	s->mmr = mmr;
	s->mem = mem;
	s->initialized = 0x59C700;
}

const uint8_t spc_ipl[0x40] = {
	0xCD, 0xEF, 0xBD, 0xE8, 0x00, 0xC6, 0x1D, 0xD0, 0xFC, 0x8F, 0xAA, 0xF4, 0x8F, 0xBB, 0xF5, 0x78,
	0xCC, 0xF4, 0xD0, 0xFB, 0x2F, 0x19, 0xEB, 0xF4, 0xD0, 0xFC, 0x7E, 0xF4, 0xD0, 0x0B, 0xE4, 0xF5,
	0xCB, 0xF4, 0xD7, 0x00, 0xFC, 0xD0, 0xF3, 0xAB, 0x01, 0x10, 0xEF, 0x7E, 0xF4, 0x10, 0xEB, 0xBA,
	0xF6, 0xDA, 0x00, 0xBA, 0xF4, 0xC4, 0xF4, 0xDD, 0x5D, 0xD0, 0xDB, 0x1F, 0x00, 0x00, 0xC0, 0xFF
};

uint8_t spc_r(spc_cpu* s, uint16_t p) {
	if((p >= 0xF0 && p <= 0xF7) || (p >= 0xFA && p <= 0xFF)) return s->mmr(p - 0xF0, 0, S_R);
	if(p >= 0xFFC0 && s->mmr(0xF1, 0, S_R) >> 7) return spc_ipl[p - 0xFFC0];
	return s->mem[p];
}

void spc_w(spc_cpu* s, uint16_t p, uint8_t d) {
	if((p >= 0xF0 && p <= 0xF7) || (p >= 0xFA && p <= 0xFF)) {
		s->mmr(p - 0xF0, d, S_W);
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

#define NUL 0
int spc_cycles[256] = { // -1: length returned by spc_eval
//	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
	2,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 0
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 1
	2,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 2
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	2,	NUL,	NUL, // 3
	2,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 4
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	2,	NUL,	NUL, // 5
	2,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 6
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 7
	2,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 8
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 9
	3,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // a
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	2,	NUL,	NUL,	NUL, // b
	3,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // c
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // d
	2,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // e
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	2,	NUL,	NUL,	NUL  // f
};

uint8_t spc_len[256] = {
//	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
	1,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 0
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 1
	1,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 2
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	1,	NUL,	NUL, // 3
	1,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 4
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	1,	NUL,	NUL, // 5
	1,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 6
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 7
	1,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 8
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // 9
	1,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // a
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	1,	NUL,	NUL,	NUL, // b
	1,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // c
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // d
	1,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL, // e
	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	NUL,	1,	NUL,	NUL,	NUL  // f
};

int spc_eval(spc_cpu* s, int* err) {
	int cycles = 0;
	switch(spc_r(s, s->PC)) {
		case 0x00:
			break;
		case 0x20:
			spc_unsetp(s);
			break;
		case 0xBC:
			spc_inc(s, s->A);
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
		case 0xC0:
			spc_unseti(s);
			break;
		case 0xE0:
			spc_unsetv(s);
			spc_unseth(s);
			break;
		case 0xFC:
			spc_inc(s, s->Y);
			break;
		default:
			printf("unimplemented opcode: 0x%02X\n", spc_r(s, s->PC));
			*err = 1;
			break;
	}
	s->PC += spc_len[spc_r(s, s->PC)];
	return cycles;
}

int spc_loop(spc_cpu* s) {
	if(s->initialized != 0x59C700) return !printf("SPENCER UNINITIALIZED\n") - 1;
	int err = 0;
	if(s->wait == 0) {
		printf("-- EVAL --\n");
		int w = spc_cycles[spc_r(s, s->PC)], c = spc_eval(s, &err);
		if(w == -1) s->wait = c;
		else s->wait = w;
		s->wait--;
	} else s->wait--;
	return err;
}
