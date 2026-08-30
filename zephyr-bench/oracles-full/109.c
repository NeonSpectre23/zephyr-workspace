int isotp_send(struct isotp_send_ctx *sctx, const struct device *can_dev,
	       const uint8_t *data, size_t len,
	       const struct isotp_msg_id *tx_addr,
	       const struct isotp_msg_id *rx_addr,
	       isotp_tx_callback_t complete_cb, void *cb_arg)
{
	sctx->data = data;
	sctx->len = len;
	sctx->is_ctx_slab = 0;
	sctx->is_net_buf = 0;

	return send(sctx, can_dev, tx_addr, rx_addr, complete_cb, cb_arg);
}