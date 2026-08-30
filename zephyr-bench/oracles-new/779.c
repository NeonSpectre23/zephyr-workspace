struct net_buf *smp_raw_dummy_get_outgoing(void)
{

	struct net_buf *nb;

	/* Decode the fragment and write the result to the global receive
	 * context.
	 */
	nb = mcumgr_dummy_process_frag(&smp_dummy_tx_ctxt, smp_send_buffer, smp_send_pos);

	return nb;
}