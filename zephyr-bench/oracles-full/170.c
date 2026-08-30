void log_thread_set(k_tid_t process_tid)
{
	if (IS_ENABLED(CONFIG_LOG_PROCESS_THREAD)) {
		__ASSERT_NO_MSG(0);
	} else {
		thread_set(process_tid);
	}
}