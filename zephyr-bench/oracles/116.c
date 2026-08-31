int log_frontend_stmesp_demux_max_utilization(void)
{
	uint32_t max;
	int rv = mpsc_pbuf_get_max_utilization(&demux.pbuf, &max);

	return rv == 0 ? max : rv;
}