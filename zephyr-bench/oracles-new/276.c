void log_frontend_stmesp_demux_timestamp(uint64_t ts)
{
	if (demux.curr == NULL) {
		return;
	}

	demux.curr->packet->timestamp = ts;
}