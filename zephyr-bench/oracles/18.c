uint32_t __weak crc24_pgp(const uint8_t *data, size_t len)
{
	return crc24_pgp_update(CRC24_PGP_INITIAL_VALUE, data, len) & CRC24_FINAL_VALUE_MASK;
}