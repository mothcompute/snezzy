#ifndef SPENCER_H
#define SPENCER_H

#include <stdint.h>
#include <stdlib.h>

// mmr flags
#define S_R 1	// read
#define S_W 0	// write
#define S_I 2	// internal (for internal emulator function)

typedef struct {
	uint8_t A, X, Y, SP, P;	// 1 byte registers
	uint16_t PC;		// 2 byte registers
	uint8_t* mem;		// 0x10000 bytes

	/* MMR arguments:
	 * 
	 * width	description
	 * -------------------------
	 * u8	| register number (0-15)
	 * u8	| data to write (if reading anything is fine)
	 * u8	| S_R to read, S_W to write
	 */
	uint8_t(*mmr)(uint8_t, uint8_t, uint8_t);
	unsigned int wait;
	unsigned int initialized;
	uint8_t sleeping;
} spc_cpu;

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

#define spc_inc(c, d) d++; spc_sbn(c, d); spc_sbz(c, d)
#define spc_trans(c, a, b) a = b; spc_sbn(c, a); spc_sbz(c, a)

#endif
