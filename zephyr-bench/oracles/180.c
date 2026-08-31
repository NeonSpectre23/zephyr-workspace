int modem_ppp_init_internal(const struct device *dev)
{
	struct modem_ppp *ppp = (struct modem_ppp *)dev->data;

	atomic_set(&ppp->state, 0);
	ring_buf_init(&ppp->transmit_rb, ppp->buf_size, ppp->transmit_buf);
	k_work_init(&ppp->send_work, modem_ppp_send_handler);
	k_work_init(&ppp->process_work, modem_ppp_process_handler);
	k_fifo_init(&ppp->tx_pkt_fifo);

#if CONFIG_MODEM_STATS
	init_buf_stats(ppp);
#endif
	return 0;
}