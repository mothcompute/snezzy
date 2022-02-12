#ifndef BARBARA_H
#define BARBARA_H

#define B_PACK_STRUCT __attribute__((packed))

typedef struct {
	uint8_t loop;
	uint8_t end;
	uint8_t shift;
	uint8_t filter;
} brr_status;

typedef struct {
	uint8_t status;
	uint8_t samples[8];
} B_PACK_STRUCT brr;

#define brr_getstatus(b, s) s->shift = b->status >> 4; s->end = b->status & 1; s->loop = (b->status >> 1) & 1; s->filter = (b->status >> 2) & 3;

#endif
