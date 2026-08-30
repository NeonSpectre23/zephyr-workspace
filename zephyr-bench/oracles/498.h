static inline uint16_t crc16_ansi(const uint8_t *src, size_t len)
{
	return crc16_reflect(0xA001, 0xffff, src, len);
}