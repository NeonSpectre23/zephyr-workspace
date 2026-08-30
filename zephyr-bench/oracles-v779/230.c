int net_pkt_write(struct net_pkt *pkt, const void *data, size_t length)
{
	NET_DBG("pkt %p data %p length %zu", pkt, data, length);

	if (data == pkt->cursor.pos && net_pkt_is_contiguous(pkt, length)) {
		return net_pkt_skip(pkt, length);
	}

	return net_pkt_cursor_operate(pkt, (void *)data, length, true, true);
}