void timing_start(void)
{
	if (atomic_inc(&started_ref) != 0) {
		return;
	}

#if defined(CONFIG_BOARD_HAS_TIMING_FUNCTIONS)
	board_timing_start();
#elif defined(CONFIG_SOC_HAS_TIMING_FUNCTIONS)
	soc_timing_start();
#else
	arch_timing_start();
#endif
}