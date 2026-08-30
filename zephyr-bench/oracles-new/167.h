static inline void net_pkt_set_ieee802154_lqi(struct net_pkt *pkt, uint8_t lqi)
{
	net_pkt_cb_ieee802154(pkt)->lqi = lqi;
}

