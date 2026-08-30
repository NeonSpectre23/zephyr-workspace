void log_flush(void)
{
	if (IS_ENABLED(CONFIG_LOG_PROCESS_THREAD)) {
		while (atomic_get(&buffered_cnt)) {
			log_thread_trigger();
			k_sleep(K_USEC(CONFIG_LOG_FLUSH_SLEEP_US));
		}
	} else {
		while (LOG_PROCESS()) {
		}
	}
}