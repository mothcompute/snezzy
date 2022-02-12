uint16_t brr_shift(uint16_t s, uint8_t f) {
	if(f <= 12) return (s << f) >> 1;
	return (s & 0x8) << 8;
}

int brr_process(brr* b, uint16_t* b) {

}
