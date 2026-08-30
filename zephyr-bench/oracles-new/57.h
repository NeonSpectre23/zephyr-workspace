static inline int64_t k_uptime_get(void)
{
	return k_ticks_to_ms_floor64(k_uptime_ticks());
}