struct net_pkt *net_pkt_alloc(k_timeout_t timeout)
#endif
{
#if NET_LOG_LEVEL >= LOG_LEVEL_DBG
	return pkt_alloc(&tx_pkts, timeout, caller, line);
#else
	return pkt_alloc(&tx_pkts, timeout);
#endif
}