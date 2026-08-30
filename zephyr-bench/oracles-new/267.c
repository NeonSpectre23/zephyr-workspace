void log_frontend_stmesp_demux_data(uint8_t *data, size_t len)
{
	if (demux.curr == NULL) {
		return;
	}

	if (demux.curr->off + len <= demux.curr->packet->hdr.total_len) {
		memcpy(&demux.curr->packet->data[demux.curr->off], data, len);
		demux.curr->off += len;
	}
}