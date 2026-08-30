void log_frontend_stmesp_demux_reset(void)
{
	sys_snode_t *node;

	while ((node = sys_slist_get(&demux.active_entries)) != NULL) {
		struct log_frontend_stmesp_demux_active_entry *entry =
			CONTAINER_OF(node, struct log_frontend_stmesp_demux_active_entry, node);
		union log_frontend_stmesp_demux_packet p = {.log = entry->packet};

		entry->packet->content_invalid = 1;
		mpsc_pbuf_commit(&demux.pbuf, p.generic);
		demux.dropped++;
		demux.curr_m_ch = M_CH_INVALID;

		k_mem_slab_free(&demux.mslab, entry);
	}
}