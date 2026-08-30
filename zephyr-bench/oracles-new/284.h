static inline uint64_t timing_cycles_get(volatile timing_t *const start,
					 volatile timing_t *const end)
{
#if defined(CONFIG_BOARD_HAS_TIMING_FUNCTIONS)
	return board_timing_cycles_get(start, end);
#elif defined(CONFIG_SOC_HAS_TIMING_FUNCTIONS)
	return soc_timing_cycles_get(start, end);
#else
	return arch_timing_cycles_get(start, end);
#endif
}