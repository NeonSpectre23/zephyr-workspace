const char *log_source_name_get(uint32_t domain_id, uint32_t source_id)
{
	if (z_log_is_local_domain(domain_id)) {
		if (IS_ENABLED(CONFIG_LOG_FMT_SECTION_STRIP)) {
			return "unknown";
		}
		if (source_id < log_src_cnt_get(domain_id)) {
			return TYPE_SECTION_START(log_const)[source_id].name;
		} else {
			return NULL;
		}
	}

	return link_source_name_get(domain_id, source_id);
}