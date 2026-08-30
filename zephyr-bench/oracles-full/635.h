static inline uint8_t can_dlc_to_bytes(uint8_t dlc)
{
	static const uint8_t dlc_table[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12,
					    16, 20, 24, 32, 48, 64};

	return dlc_table[MIN(dlc, ARRAY_SIZE(dlc_table) - 1)];
}