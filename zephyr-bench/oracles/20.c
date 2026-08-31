uint32_t crc24q_rtcm3(const uint8_t *data, size_t len)
{
	uint32_t crc = 0;

	for (uint32_t i = 0; i < len; ++i) {
		crc = ((crc << 8) & 0xFFFFFF) ^ crc24q[data[i] ^ (crc >> 16)];
	}

	return crc;
}