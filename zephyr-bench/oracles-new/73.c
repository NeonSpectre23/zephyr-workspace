int cpu_load_get(bool reset)
{
	uint64_t idle_us;
	uint64_t now = IS_ENABLED(CONFIG_TIMER_HAS_64BIT_CYCLE_COUNTER) ?
		k_cycle_get_64() : k_cycle_get_32();
	uint64_t total = now - cyc_start;
	uint64_t total_us = k_cyc_to_us_floor64(total);
	uint32_t res;
	uint64_t active_us;

	if (IS_ENABLED(CONFIG_CPU_LOAD_USE_COUNTER)) {
		if (ticks_idle > (uint64_t)UINT32_MAX) {
			return -ERANGE;
		}
		idle_us = counter_ticks_to_us(counter, (uint32_t)ticks_idle);
	} else {
		idle_us = k_cyc_to_us_floor64(ticks_idle);
	}

	idle_us = MIN(idle_us, total_us);
	active_us = total_us - idle_us;

	res = (uint32_t)((1000 * active_us) / total_us);

	if (reset) {
		cyc_start = now;
		ticks_idle = 0;
	}

	return res;
}