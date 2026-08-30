uint32_t log_frontend_stmesp_demux_get_dropped(void)
{
	uint32_t rv = demux.dropped;

	demux.dropped = 0;

	return rv;
}