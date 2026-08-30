const char *log_domain_name_get(uint32_t domain_id)
{
	if (z_log_is_local_domain(domain_id)) {
		return CONFIG_LOG_DOMAIN_NAME;
	}

	return link_domain_name_get(domain_id);
}