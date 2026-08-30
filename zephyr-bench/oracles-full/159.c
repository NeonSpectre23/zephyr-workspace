void log_frontend_stmesp_demux_packet_end(void)
{
	if (demux.curr == NULL) {
		return;
	}

	union log_frontend_stmesp_demux_packet p = {.log = demux.curr->packet};

	if (demux.curr->off != demux.curr->packet->hdr.total_len) {
		demux.curr->packet->content_invalid = 1;
		demux.dropped++;
	}

	mpsc_pbuf_commit(&demux.pbuf, p.generic);

	sys_slist_find_and_remove(&demux.active_entries, &demux.curr->node);
	k_mem_slab_free(&demux.mslab, demux.curr);
	demux.curr = NULL;
}