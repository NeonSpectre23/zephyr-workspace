union log_frontend_stmesp_demux_packet log_frontend_stmesp_demux_claim(void)
{
	union log_frontend_stmesp_demux_packet p;

	/* Discard any invalid packets. */
	while ((p.rgeneric = mpsc_pbuf_claim(&demux.pbuf)) != NULL) {
		if (p.generic_packet->content_invalid) {
			mpsc_pbuf_free(&demux.pbuf, p.rgeneric);
		} else {
			break;
		}
	}

	return p;
}