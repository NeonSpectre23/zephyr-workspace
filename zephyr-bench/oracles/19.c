uint32_t __weak crc24_pgp_update(uint32_t crc, const uint8_t *data, size_t len)
{
	int i;

	while (len--) {
		crc ^= (*data++) << 16;
		for (i = 0; i < 8; i++) {
			crc <<= 1;
			if (crc & 0x01000000) {
				crc ^= CRC24_PGP_POLY;
			}
		}
	}

	return crc;
}