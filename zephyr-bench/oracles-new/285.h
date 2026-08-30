static inline uint64_t timing_cycles_to_ns(uint64_t cycles)
{
#if defined(CONFIG_BOARD_HAS_TIMING_FUNCTIONS)
	return board_timing_cycles_to_ns(cycles);
#elif defined(CONFIG_SOC_HAS_TIMING_FUNCTIONS)
	return soc_timing_cycles_to_ns(cycles);
#else
	return arch_timing_cycles_to_ns(cycles);
#endif
}