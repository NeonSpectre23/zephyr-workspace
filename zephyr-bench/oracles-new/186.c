int isotp_bind(struct isotp_recv_ctx *rctx, const struct device *can_dev,
	       const struct isotp_msg_id *rx_addr,
	       const struct isotp_msg_id *tx_addr,
	       const struct isotp_fc_opts *opts,
	       k_timeout_t timeout)
{
	can_mode_t cap;
	int ret;

	__ASSERT(rctx, "rctx is NULL");
	__ASSERT(can_dev, "CAN device is NULL");
	__ASSERT(rx_addr && tx_addr, "RX or TX addr is NULL");
	__ASSERT(opts, "OPTS is NULL");

	rctx->can_dev = can_dev;
	rctx->rx_addr = *rx_addr;
	rctx->tx_addr = *tx_addr;
	k_fifo_init(&rctx->fifo);

	__ASSERT(opts->stmin < ISOTP_STMIN_MAX, "STmin limit");
	__ASSERT(opts->stmin <= ISOTP_STMIN_MS_MAX ||
		 opts->stmin >= ISOTP_STMIN_US_BEGIN, "STmin reserved");

	rctx->opts = *opts;
	rctx->state = ISOTP_RX_STATE_WAIT_FF_SF;

	if ((rx_addr->flags & ISOTP_MSG_FDF) != 0 || (tx_addr->flags & ISOTP_MSG_FDF) != 0) {
		ret = can_get_capabilities(can_dev, &cap);
		if (ret != 0 || (cap & CAN_MODE_FD) == 0) {
			LOG_ERR("CAN controller does not support FD mode");
			return ISOTP_N_ERROR;
		}
	}

	LOG_DBG("Binding to addr: 0x%x. Responding on 0x%x",
		rctx->rx_addr.ext_id, rctx->tx_addr.ext_id);

	rctx->buf = net_buf_alloc_fixed(&isotp_rx_sf_ff_pool, timeout);
	if (!rctx->buf) {
		LOG_ERR("No buffer for FF left");
		return ISOTP_NO_NET_BUF_LEFT;
	}

	ret = add_ff_sf_filter(rctx);
	if (ret) {
		LOG_ERR("Can't add filter for binding");
		net_buf_unref(rctx->buf);
		rctx->buf = NULL;
		return ret;
	}

	k_work_init(&rctx->work, receive_work_handler);
	k_timer_init(&rctx->timer, receive_timeout_handler, NULL);

	return ISOTP_N_OK;
}