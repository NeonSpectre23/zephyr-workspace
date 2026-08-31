void log_frontend_stmesp_demux_channel(uint16_t id)
{
	if (skip) {
		return;
	}

	if (id == CONFIG_LOG_FRONTEND_STMESP_FLUSH_PORT_ID) {
		/* Flushing data that shall be discarded. */
		goto bail;
	}

	demux.curr_m_ch &= ~C_ID_MASK;
	demux.curr_m_ch |= id;

	sys_snode_t *node;

	SYS_SLIST_FOR_EACH_NODE(&demux.active_entries, node) {
		struct log_frontend_stmesp_demux_active_entry *entry =
			CONTAINER_OF(node, struct log_frontend_stmesp_demux_active_entry, node);

		if (entry->m_ch == demux.curr_m_ch) {
			demux.curr = entry;
			return;
		}
	}

bail:
	demux.curr = NULL;
}