static inline timing_t timing_counter_get(void)
{
#if defined(CONFIG_BOARD_HAS_TIMING_FUNCTIONS)
	return board_timing_counter_get();
#elif defined(CONFIG_SOC_HAS_TIMING_FUNCTIONS)
	return soc_timing_counter_get();
#else
	return arch_timing_counter_get();
#endif
}