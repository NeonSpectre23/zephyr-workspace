uint32_t log_src_cnt_get(uint32_t domain_id)
{
	if (z_log_is_local_domain(domain_id)) {
		return z_log_sources_count();
	}

	return link_source_count(domain_id);
}