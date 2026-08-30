void log_core_init(void)
{
	panic_mode = false;
	dropped_cnt = 0;
	buffered_cnt = 0;

	if (IS_ENABLED(CONFIG_LOG_FRONTEND)) {
		log_frontend_init();

		if (IS_ENABLED(CONFIG_LOG_RUNTIME_FILTERING)) {
			for (uint16_t s = 0; s < log_src_cnt_get(0); s++) {
				log_frontend_filter_set(s, CONFIG_LOG_MAX_LEVEL);
			}
		}

		if (IS_ENABLED(CONFIG_LOG_FRONTEND_ONLY)) {
			return;
		}
	}

	/* Set default timestamp. */
#ifdef CONFIG_LOG_TIMESTAMP_USE_REALTIME
	log_set_timestamp_func(default_rt_get_timestamp, 1000U);
#else
	if (sys_clock_hw_cycles_per_sec() > 1000000) {
		log_set_timestamp_func(default_lf_get_timestamp, 1000U);
	} else {
		uint32_t freq = IS_ENABLED(CONFIG_LOG_TIMESTAMP_64BIT) ?
			CONFIG_SYS_CLOCK_TICKS_PER_SEC : sys_clock_hw_cycles_per_sec();
		log_set_timestamp_func(default_get_timestamp, freq);
	}
#endif /* CONFIG_LOG_TIMESTAMP_USE_REALTIME */

	if (IS_ENABLED(CONFIG_LOG_MODE_DEFERRED)) {
		z_log_msg_init();
	}

	if (IS_ENABLED(CONFIG_LOG_RUNTIME_FILTERING)) {
		z_log_runtime_filters_init();
	}

	STRUCT_SECTION_FOREACH(log_backend, backend) {
		uint32_t id;
		/* As first slot in filtering mask is reserved, backend ID has offset.*/
		id = LOG_FILTER_FIRST_BACKEND_SLOT_IDX;
		id += backend - log_backend_get(0);
		log_backend_id_set(backend, id);
	}
}