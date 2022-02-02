#include "layla.h"

uint8_t (*ll_read)(ll_cpu* c, uint32_t addr) = NULL;
void (*ll_write)(ll_cpu* c, uint32_t addr, uint8_t b) = NULL;

int ll_init_status = 0;

void ll_reg_read(uint8_t(*rdp)(ll_cpu* c, uint32_t addr)) {
	ll_read = rdp;
	ll_init_status |= 2;
	if(ll_init_status == 6) ll_init_status = 1;
}

void ll_reg_write(void(*wrp)(ll_cpu* c, uint32_t addr, uint8_t b)) {
	ll_write = wrp;
	ll_init_status |= 4;
	if(ll_init_status == 6) ll_init_status = 1;
}

uint16_t ll_read16(ll_cpu* c, uint32_t addr) {
	// TODO
	return 0;
}

void ll_write16(ll_cpu* c, uint32_t addr, uint16_t w) {
	// TODO
}

uint16_t llnop(ll_cpu* c) {}

uint16_t ll18(ll_cpu* c) {
	c->P.f.C = 0;
}

uint16_t ll1b(ll_cpu* c) {
	if(c->e) {
		if(c->SP.D.H != 1) exit(printf("LINE %s ERR\n", __LINE__));
		c->SP.D.L = c->A.D.L;
	}
	else c->SP.F = c->A.F;
}

uint16_t ll38(ll_cpu* c) {
	c->P.f.C = 1;
}

uint16_t ll3b(ll_cpu* c) {
	c->A.F = c->SP.F;
	ll_setnz16(SP);
}

uint16_t ll58(ll_cpu* c) {
	c->P.f.I = 0;
}

uint16_t ll5b(ll_cpu* c) {
	c->D.F = c->A.F;
	ll_setnz16(D);
}

uint16_t ll78(ll_cpu* c) {
	c->P.f.I = 1;
}

uint16_t ll7b(ll_cpu* c) {
	c->A.F = c->D.F;
	ll_setnz16(A);
}

uint16_t ll98(ll_cpu* c) {
	ll_trans(Y, A);
}

uint16_t ll9a(ll_cpu* c) {
	// ll_trans sans status
	if(c->P.f.XB) c->SP.D.L = c->X.D.L;
	// TODO needed?
	if(!c->e && c->P.f.XB) c->SP.D.H = 0;
	else c->SP.F = c->X.F;
}

uint16_t ll9b(ll_cpu* c) {
	ll_trans(X, Y);
}

uint16_t lla8(ll_cpu* c) {
	ll_trans(A, Y);
}

// `llaa, you got me on my knees`
uint16_t llaa(ll_cpu* c) {
	ll_trans(A, X);
}

uint16_t llb8(ll_cpu* c) {
	c->P.f.V = 0;
}

uint16_t llba(ll_cpu* c) {
	ll_trans(SP, X);
}

uint16_t lld8(ll_cpu* c) {
	c->P.f.D = 0;
}

uint16_t lleb(ll_cpu* c) {
	ll_swap(c->A.D.H, c->A.D.L);
	ll_setnz8(A);
}

uint16_t llf8(ll_cpu* c) {
	c->P.f.D = 1;
	printf("DECIMAL UNIMPLEMENTED\n");
}

uint16_t llfb(ll_cpu* c) {
	c->e = c->P.f.C;
	if(c->e) {
		c->P.f.M = 1;
		c->P.f.XB = 1;
		c->X.D.H = 0;
		c->Y.D.H = 0;
		c->SP.D.H = 1;
	}
}

#define LL_E 0
// if negative, called opcode will return the number of cycles. else, use this
int ll_cycles[256] = {
//(YX)	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	// 0
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	// 1
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	// 2
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	2,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	// 3
	LL_E,	LL_E,	2,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	// 4
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	// 5
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	// 6
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	2,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	// 7
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	// 8
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	2,	2,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	// 9
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	2,	LL_E,	2,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	// a
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	2,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	// b
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	// c
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	// d
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	2,	3,	LL_E,	LL_E,	LL_E,	LL_E,	// e
	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	LL_E,	2,	LL_E,	LL_E,	2,	LL_E,	LL_E,	LL_E,	LL_E	// f
};

ll_opcode ll_opcodes[256] = {
//(YX)	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	// 0
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	&ll18,	NULL,	NULL,	&ll1b,	NULL,	NULL,	NULL,	NULL,	// 1
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	// 2
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	&ll38,	NULL,	NULL,	&ll3b,	NULL,	NULL,	NULL,	NULL,	// 3
	NULL,	NULL,	&llnop,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	// 4
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	&ll58,	NULL,	NULL,	&ll5b,	NULL,	NULL,	NULL,	NULL,	// 5
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	// 6
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	&ll78,	NULL,	NULL,	&ll7b,	NULL,	NULL,	NULL,	NULL,	// 7
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	// 8
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	&ll98,	&ll9b,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	// 9
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	&lla8,	NULL,	&llaa,	NULL,	NULL,	NULL,	NULL,	NULL,	// a
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	&llb8,	NULL,	&llba,	NULL,	NULL,	NULL,	NULL,	NULL,	// b
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	// c
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	&lld8,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	// d
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	&llnop,	&lleb,	NULL,	NULL,	NULL,	NULL,	// e
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	&llf8,	NULL,	NULL,	&llfb,	NULL,	NULL,	NULL,	NULL	// f
};
