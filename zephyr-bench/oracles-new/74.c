void cpu_load_log_control(bool enable)
{
	if (CONFIG_CPU_LOAD_LOG_PERIODICALLY == 0) {
		return;
	}
	if (enable) {
		k_timer_start(&cpu_load_timer, K_MSEC(CONFIG_CPU_LOAD_LOG_PERIODICALLY),
			      K_MSEC(CONFIG_CPU_LOAD_LOG_PERIODICALLY));
	} else {
		k_timer_stop(&cpu_load_timer);
	}
}