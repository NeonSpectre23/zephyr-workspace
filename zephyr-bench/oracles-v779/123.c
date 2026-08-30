void log_frontend_stmesp_demux_free(union log_frontend_stmesp_demux_packet packet)
{
	mpsc_pbuf_free(&demux.pbuf, packet.rgeneric);
}