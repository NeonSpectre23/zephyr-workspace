static inline bool log_data_pending(void)
{
	return IS_ENABLED(CONFIG_LOG_MODE_DEFERRED) ? z_log_msg_pending() : false;
}