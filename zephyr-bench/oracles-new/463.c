int net_pkt_read(struct net_pkt *pkt, void *data, size_t length)
{
	NET_DBG("pkt %p data %p length %zu", pkt, data, length);

	return net_pkt_cursor_operate(pkt, data, length, true, false);
}