#include <stdlib.h>
#include <stdio.h>
#include "layla.c"
int main() {
	printf("sizeof ll_status: %u\n", sizeof(ll_status));
	ll_reg b;
	b.F = 0xFFEE;
	printf("%02X\n", b.D.H);
}
