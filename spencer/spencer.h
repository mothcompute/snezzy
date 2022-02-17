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

	// the void*s in the function declarations are spc_cpus
	
	/* MMR arguments:
	 * 
	 * width	description
	 * -------------------------
	 * u8	| register number (0-15)
	 * u8	| data to write (if reading anything is fine)
	 * u8	| S_R to read, S_W to write, or with S_I to prevent resetting data
	 */
	uint8_t(*mmr)(void*, uint8_t, uint8_t, uint8_t);
	void(*stop)(void*);
	unsigned int wait;
	unsigned int initialized;
	uint8_t stp;
	uint8_t use;
	uint8_t con;
} spc_cpu;

int spc_loop(spc_cpu*);
void spc_init(spc_cpu*, void*, uint8_t(*)(spc_cpu*, uint8_t, uint8_t, uint8_t), void(*)(spc_cpu*));

#endif
