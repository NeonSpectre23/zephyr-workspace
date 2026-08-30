uint32_t z_log_links_activate(uint32_t active_mask, uint8_t *offset)
{
	uint32_t mask = 0x1;
	uint32_t out_mask = 0;

	/* Initiate offset to 1. */
	if (*offset == 0) {
		*offset = 1;
	}

	STRUCT_SECTION_FOREACH(log_link, link) {
		if (active_mask & mask) {
			int err = log_link_activate(link);

			if (err == 0) {
				uint8_t domain_cnt = log_link_domains_count(link);

				link->ctrl_blk->domain_offset = *offset;
				link->ctrl_blk->domain_cnt = domain_cnt;
				*offset += domain_cnt;
				if (IS_ENABLED(CONFIG_LOG_RUNTIME_FILTERING)) {
					link_filters_init(link);
					backends_link_init(link);
				}
			} else {
				__ASSERT_NO_MSG(err == -EINPROGRESS);
				out_mask |= mask;
			}
		}

		mask <<= 1;
	}

	return out_mask;
}