void log_frontend_stmesp_demux_major(uint16_t id)
{
	for (int i = 0; i < demux.m_ids_cnt; i++) {
		if (id == demux.m_ids[i]) {
			sys_snode_t *node;

			demux.curr_m_ch = id << M_ID_OFF;
			demux.curr = NULL;

			SYS_SLIST_FOR_EACH_NODE(&demux.active_entries, node) {
				struct log_frontend_stmesp_demux_active_entry *entry =
					CONTAINER_OF(node,
						struct log_frontend_stmesp_demux_active_entry,
						node);

				if (entry->m_ch == demux.curr_m_ch) {
					demux.curr = entry;
					break;
				}
			}
			skip = false;
			return;
		}
	}

	skip = true;
}