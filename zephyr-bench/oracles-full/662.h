static inline uint32_t k_uptime_get_32(void)
{
	return (uint32_t)k_uptime_get();
}