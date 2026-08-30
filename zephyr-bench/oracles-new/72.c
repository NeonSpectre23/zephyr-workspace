int cpu_load_cb_reg(cpu_load_cb_t cb, uint8_t threshold_percent)
{
	if (threshold_percent > 100) {
		return -EINVAL;
	}

	cpu_load_threshold_percent = threshold_percent;
	load_cb = cb;
	return 0;
}