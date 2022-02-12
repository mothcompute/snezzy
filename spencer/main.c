#include <spencer.c>

uint8_t mmr(spc_cpu* s, uint8_t reg, uint8_t d, uint8_t rw) {
	
}

void stop(spc_cpu* s) {

}

int main(int argc, char** argv) {
	spc_cpu s;
	spc_init(&s, malloc(0x10000), &mmr, &stop);
	// TODO copy in new spc data
	while(1) {
		printf(	"WAIT: %u\n"
			"\tPC:   0x%04X\n"
			"\tSP:   0x%02X\n"
			"\tA:    0x%02X\n"
			"\tX:    0x%02X\n"
			"\tY:    0x%02X\n"
			,
			s.wait,
			s.PC,
			s.SP,
			s.A,
			s.X,
			s.Y
		);
		int r = spc_loop(&s);
		if(r) exit(printf("error\n"));
	}
	return printf("stub\n");
}
