uint8_t __weak crc8(const uint8_t *src, size_t len, uint8_t polynomial, uint8_t initial_value,
		    bool reversed)
{
	uint8_t crc = initial_value;
	size_t i, j;

	for (i = 0; i < len; i++) {
		crc ^= src[i];

		for (j = 0; j < 8; j++) {
			if (reversed) {
				if ((crc & 0x01) != 0) {
					crc = (crc >> 1) ^ polynomial;
				} else {
					crc >>= 1;
				}
			} else {
				if ((crc & 0x80) != 0) {
					crc = (crc << 1) ^ polynomial;
				} else {
					crc <<= 1;
				}
			}
		}
	}

	return crc;
}