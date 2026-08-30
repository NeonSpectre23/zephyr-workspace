struct net_buf *smp_dummy_get_outgoing(void)
{
	return smp_dummy_process_frag_outgoing(smp_send_buffer, smp_send_pos);
}