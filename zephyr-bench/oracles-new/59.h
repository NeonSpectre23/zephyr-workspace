static inline uint32_t k_uptime_seconds(void)
{
	return k_ticks_to_sec_floor32(k_uptime_ticks());
}