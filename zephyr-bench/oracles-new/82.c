uint32_t __weak crc32_c(uint32_t crc, const uint8_t *data, size_t len, bool first_pkt,
			bool last_pkt)
{
	if (first_pkt) {
		crc = CRC32C_INIT;
	}

	for (size_t i = 0; i < len; i++) {
		crc = crc32c_table[(crc ^ data[i]) & 0x0F] ^ (crc >> 4);
		crc = crc32c_table[(crc ^ ((uint32_t)data[i] >> 4)) & 0x0F] ^ (crc >> 4);
	}

	return last_pkt ? (crc ^ CRC32C_XOR_OUT) : crc;
}