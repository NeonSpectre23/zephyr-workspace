uint8_t __weak crc4_ti(uint8_t seed, const uint8_t *src, size_t len)
{
	static const uint8_t lookup[8] = {0x03, 0x65, 0xcf, 0xa9, 0xb8, 0xde, 0x74, 0x12};
	uint8_t index;

	for (size_t i = 0; i < len; i++) {
		for (size_t j = 0U; j < 2U; j++) {
			index = seed ^ ((src[i] >> (4 * (1 - j))) & 0xf);
			seed = (lookup[index >> 1] >> (1 - (index & 1)) * 4) & 0xf;
		}
	}

	return seed;
}