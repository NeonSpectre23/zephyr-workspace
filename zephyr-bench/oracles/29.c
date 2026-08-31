uint8_t __weak crc8_rohc(uint8_t val, const void *buf, size_t cnt)
{
	size_t i;
	const uint8_t *p = buf;

	for (i = 0; i < cnt; i++) {
		val ^= p[i];
		val = (val >> 4) ^ crc8_rohc_small_table[val & 0x0f];
		val = (val >> 4) ^ crc8_rohc_small_table[val & 0x0f];
	}
	return val;
}