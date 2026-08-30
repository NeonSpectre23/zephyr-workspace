void smp_packet_free(struct net_buf *nb)
{
	net_buf_unref(nb);
}