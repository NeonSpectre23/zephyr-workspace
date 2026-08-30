int log_frontend_stmesp_demux_packet_start(uint32_t *data, uint64_t *ts)
{
	if (skip) {
		return 0;
	}

	struct log_frontend_stmesp_demux_active_entry *entry;
	union log_frontend_stmesp_demux_packet p;
	int err;

	if (demux.curr_m_ch == M_CH_INVALID) {
		return -EINVAL;
	}

	if (demux.curr_m_ch == M_CH_HW_EVENT) {
		/* HW event */
		log_frontend_stmesp_demux_hw_event(ts, (uint8_t)*data);

		return 1;
	}

	uint16_t ch = demux.curr_m_ch & C_ID_MASK;
	uint16_t m = get_major_id(demux.curr_m_ch >> M_ID_OFF);

	if (IS_ENABLED(CONFIG_LOG_FRONTEND_STMESP_TURBO_LOG) &&
	    (ch == CONFIG_LOG_FRONTEND_STPESP_TURBO_SOURCE_PORT_ID)) {
		struct log_frontend_stmesp_coproc_sources *src =
			&demux.coproc_sources[demux.m_ids[m] == FLPR_M_ID ? 0 : 1];

		if (src->data_cnt >= 2) {
			/* Unexpected packet. */
			return -EINVAL;
		}

		src->m_id = m;
		src->raw_data.data[src->data_cnt++] = (uintptr_t)*data;
		return 0;
	}

	if (demux.curr != NULL) {
		/* Previous package was incompleted. Finish it and potentially
		 * mark as incompleted if not all data is received.
		 */
		log_frontend_stmesp_demux_packet_end();
		return -EINVAL;
	}

	if (ch >= CONFIG_LOG_FRONTEND_STMESP_TP_CHAN_BASE) {
		/* Trace point */
		if (ch >= CONFIG_LOG_FRONTEND_STMESP_TURBO_LOG_BASE) {
			store_turbo_log1(m, ch, ts, *data);
		} else {
			store_tracepoint(m, ch, ts, data);
		}

		return 1;
	}

	union log_frontend_stmesp_demux_header hdr = {.raw = *data};
	uint32_t pkt_len = hdr.log.total_len + offsetof(struct log_frontend_stmesp_demux_log, data);
	uint32_t wlen = calc_wlen(pkt_len);
	uint32_t now = k_uptime_get_32();

	garbage_collector(now);
	err = k_mem_slab_alloc(&demux.mslab, (void **)&entry, K_NO_WAIT);
	if (err < 0) {
		goto on_nomem;
	}

	entry->m_ch = demux.curr_m_ch;
	entry->off = 0;
	p.generic = mpsc_pbuf_alloc(&demux.pbuf, wlen, K_NO_WAIT);
	if (p.generic == NULL) {
		k_mem_slab_free(&demux.mslab, entry);
		goto on_nomem;
	}

	entry->packet = p.log;
	entry->packet->type = LOG_FRONTEND_STMESP_DEMUX_TYPE_LOG;
	entry->packet->content_invalid = 0;
	if (ts) {
		entry->packet->timestamp = *ts;
	}
	entry->packet->hdr = hdr.log;
	entry->packet->hdr.major = m;
	entry->ts = now;
	demux.curr = entry;
	sys_slist_append(&demux.active_entries, &entry->node);

	return 0;

on_nomem:
	demux.curr = NULL;
	demux.dropped++;
	return -ENOMEM;
}