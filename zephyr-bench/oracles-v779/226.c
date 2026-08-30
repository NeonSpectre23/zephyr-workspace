struct net_pkt *net_pkt_alloc_with_buffer(struct net_if *iface,
					  size_t size,
					  net_sa_family_t family,
					  enum net_ip_protocol proto,
					  k_timeout_t timeout)
#endif
{
#if NET_LOG_LEVEL >= LOG_LEVEL_DBG
	return pkt_alloc_with_buffer(&tx_pkts, iface, size, family,
				     proto, timeout, caller, line);
#else
	return pkt_alloc_with_buffer(&tx_pkts, iface, size, family,
				     proto, timeout);
#endif
}