#ifndef LAYLA_H
#define LAYLA_H

#include <stdint.h>

#define PACK_STRUCT __attribute__((packed))
#define ll_swap(X, Y) X ^= Y ^= X ^= Y

typedef struct {
	uint8_t C  : 1;
	uint8_t Z  : 1;
	uint8_t I  : 1;
	uint8_t D  : 1;
	uint8_t XB : 1;
	uint8_t M  : 1;
	uint8_t V  : 1;
	uint8_t N  : 1;
} PACK_STRUCT ll_status_internal;

typedef union {
	uint8_t byte;
	ll_status_internal f;
} ll_status;

typedef struct {
	uint8_t L;
	uint8_t H;
} ll_reg_HL;

typedef union {
	uint16_t F;
	ll_reg_HL D;
} ll_reg;

typedef struct {
	// bools? whats that, a bowl spoon?
	uint8_t e : 1;
	uint8_t DB;
	uint8_t PB;
	ll_status P;
	ll_reg A;
	ll_reg X;
	ll_reg Y;
	ll_reg D;
	ll_reg SP;
	ll_reg PC;
	void* data;
} ll_cpu;

typedef uint16_t(*ll_opcode)(ll_cpu*);

#define ll_setnz16(DEST) \
	c->P.f.N = c->DEST.F >> 15; \
	c->P.f.Z = (c->DEST.F == 0)

#define ll_setnz8(DEST) \
	c->P.f.N = c->DEST.D.L >> 7; \
	c->P.f.Z = (c->DEST.D.L == 0)

#define ll_trans(SRC, DEST) \
	if(c->P.f.XB) { \
		c->DEST.D.L = c->SRC.D.L; \
		ll_setnz8(DEST); \
	} else { \
		c->DEST.F = c->SRC.F; \
		ll_setnz16(DEST); \
	}

#endif
