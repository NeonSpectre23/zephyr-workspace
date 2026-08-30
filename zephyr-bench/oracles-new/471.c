int notify_new_rx_frame(struct net_pkt *pkt)
{
	k_fifo_put(&rx_pkt_fifo, pkt);
	set_pending_event(PENDING_EVENT_FRAME_RECEIVED);

	return 0;
}