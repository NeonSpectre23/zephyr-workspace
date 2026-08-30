void isotp_unbind(struct isotp_recv_ctx *rctx)
{
	struct net_buf *buf;

	if (rctx->filter_id >= 0 && rctx->can_dev) {
		can_remove_rx_filter(rctx->can_dev, rctx->filter_id);
	}

	k_timer_stop(&rctx->timer);

	sys_slist_find_and_remove(&global_ctx.ff_sf_alloc_list, &rctx->alloc_node);
	sys_slist_find_and_remove(&global_ctx.alloc_list, &rctx->alloc_node);

	rctx->state = ISOTP_RX_STATE_UNBOUND;

	while ((buf = k_fifo_get(&rctx->fifo, K_NO_WAIT))) {
		net_buf_unref(buf);
	}

	k_fifo_cancel_wait(&rctx->fifo);

	if (rctx->buf) {
		net_buf_unref(rctx->buf);
	}

	LOG_DBG("Unbound");
}