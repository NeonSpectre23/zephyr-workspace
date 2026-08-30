void timing_init(void)
{
	if (has_inited) {
		return;
	}

#if defined(CONFIG_BOARD_HAS_TIMING_FUNCTIONS)
	board_timing_init();
#elif defined(CONFIG_SOC_HAS_TIMING_FUNCTIONS)
	soc_timing_init();
#else
	arch_timing_init();
#endif

	has_inited = true;
}