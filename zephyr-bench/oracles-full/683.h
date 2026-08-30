static inline uint8_t log_domains_count(void)
{
	return 1 + (IS_ENABLED(CONFIG_LOG_MULTIDOMAIN) ? z_log_ext_domain_count() : 0);
}