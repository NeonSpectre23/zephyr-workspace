void log_thread_trigger(void)
{
	if (IS_ENABLED(CONFIG_LOG_MODE_IMMEDIATE)) {
		return;
	}

	k_timer_stop(&log_process_thread_timer);
	k_sem_give(&log_process_thread_sem);
}