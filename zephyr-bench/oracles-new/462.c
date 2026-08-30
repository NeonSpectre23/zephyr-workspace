struct net_buf *net_pkt_get_reserve_tx_data(size_t min_len, k_timeout_t timeout)
{
	return net_pkt_get_reserve_data(&tx_bufs, min_len, timeout);
}