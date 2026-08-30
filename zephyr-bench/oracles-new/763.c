void smp_client_buf_free(struct net_buf *nb)
{
	smp_packet_free(nb);
}