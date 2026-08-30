void net_pkt_append_buffer(struct net_pkt *pkt, struct net_buf *buffer)
{
	if (!pkt->buffer) {
		pkt->buffer = buffer;
		net_pkt_cursor_init(pkt);
	} else {
		net_buf_frag_insert(net_buf_frag_last(pkt->buffer), buffer);
	}
}