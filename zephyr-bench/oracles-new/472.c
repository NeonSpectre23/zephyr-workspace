int notify_new_tx_frame(struct net_pkt *pkt)
{
	k_fifo_put(&tx_pkt_fifo, pkt);
	set_pending_event(PENDING_EVENT_FRAME_TO_SEND);

	return 0;
}