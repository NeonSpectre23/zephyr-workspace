int log_set_timestamp_func(log_timestamp_get_t timestamp_getter, uint32_t freq)
{
	if (timestamp_getter == NULL) {
		return -EINVAL;
	}

	timestamp_func = timestamp_getter;
	timestamp_freq = freq;
	if (CONFIG_LOG_PROCESSING_LATENCY_US) {
		proc_latency = (freq * CONFIG_LOG_PROCESSING_LATENCY_US) / 1000000;
	}

	if (IS_ENABLED(CONFIG_LOG_OUTPUT)) {
		log_output_timestamp_freq_set(freq);
	}

	return 0;
}