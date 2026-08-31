void net_pkt_cursor_init(struct net_pkt *pkt)
{
	pkt->cursor.buf = pkt->buffer;
	if (pkt->cursor.buf) {
		pkt->cursor.pos = pkt->cursor.buf->data;
	} else {
		pkt->cursor.pos = NULL;
	}
}