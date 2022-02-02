#include <stdio.h>
#include "layla.c"

int main() {
	int c = 0;
	for(int i = 0; i < 256; i++) c+=(!!(long)ll_opcodes[i]);
	printf("%i opcodes complete (%f%% to native mode success!)\n", c, 0.390625 * c);
}
