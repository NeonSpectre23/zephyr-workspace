static inline int net_pkt_write_u8(struct net_pkt *pkt, uint8_t data)
{
	return net_pkt_write(pkt, &data, sizeof(uint8_t));
}