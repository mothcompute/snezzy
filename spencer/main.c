#include <spencer.c>

uint8_t mmr(spc_cpu* s, uint8_t reg, uint8_t d, uint8_t rw) {
	//return 0;
}

void stop(spc_cpu* s) {

}

int main(int argc, char** argv) {
	spc_cpu s;
	spc_init(&s, malloc(0x10000), &mmr, &stop);
	memset(s.mem, 0, 0x10000);
	unsigned int icount = 0;
	// TODO copy in new spc data
	while(1) {
		printf(	"WAIT: %u\n"
			"\tPC:   0x%04X\n"
			"\tSP:   0x%02X\n"
			"\tA:    0x%02X\n"
			"\tX:    0x%02X\n"
			"\tY:    0x%02X\n"
			"\tP:    %c%c%c%c%c%c%c%c\n"
			"\ticount %u\n"
			,
			s.wait,
			s.PC,
			s.SP,
			s.A,
			s.X,
			s.Y,
			s.P >> 7 ? 'N' : '.',
			s.P >> 6 ? 'V' : '.',
			s.P >> 5 ? 'P' : '.',
			s.P >> 4 ? 'B' : '.',
			s.P >> 3 ? 'H' : '.',
			s.P >> 2 ? 'I' : '.',
			s.P >> 1 ? 'Z' : '.',
			s.P >> 0 ? 'C' : '.',
			icount
		);
		int r = spc_loop(&s);
		icount++;
		if(r < 0) exit(printf("error\n"));
	}
	return printf("stub\n");
}
