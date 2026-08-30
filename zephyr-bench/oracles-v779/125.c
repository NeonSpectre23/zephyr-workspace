int log_frontend_stmesp_demux_init(const struct log_frontend_stmesp_demux_config *config)
{
	int err;
	static const struct mpsc_pbuf_buffer_config pbuf_config = {
		.buf = buffer,
		.size = ARRAY_SIZE(buffer),
		.notify_drop = notify_drop,
		.get_wlen = get_wlen,
		.flags = MPSC_PBUF_MODE_OVERWRITE |
			 (IS_ENABLED(CONFIG_LOG_FRONTEND_STMESP_DEMUX_MAX_UTILIZATION) ?
				MPSC_PBUF_MAX_UTILIZATION : 0)};

	memset(buffer, 0, sizeof(buffer));
	mpsc_pbuf_init(&demux.pbuf, &pbuf_config);

	sys_slist_init(&demux.active_entries);

	if (config->m_ids_cnt > BIT(3)) {
		return -EINVAL;
	}

	demux.m_ids = config->m_ids;
	demux.m_ids_cnt = config->m_ids_cnt;
	demux.dropped = 0;
	demux.curr_m_ch = M_CH_INVALID;
	demux.curr = NULL;
	demux.source_ids = config->source_id_buf;
	demux.source_id_len = config->source_id_buf_len / config->m_ids_cnt - 1;

	err = k_mem_slab_init(&demux.mslab, slab_buf,
			      sizeof(struct log_frontend_stmesp_demux_active_entry),
			      NUM_OF_ACTIVE);

	return err;
}