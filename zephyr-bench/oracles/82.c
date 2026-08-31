int isotp_recv_net(struct isotp_recv_ctx *rctx, struct net_buf **buffer, k_timeout_t timeout)
{
	struct net_buf *buf;
	int ret;

	buf = k_fifo_get(&rctx->fifo, timeout);
	if (!buf) {
		ret = rctx->error_nr ? rctx->error_nr : ISOTP_RECV_TIMEOUT;
		rctx->error_nr = 0;

		return ret;
	}

	*buffer = buf;

	return *(uint32_t *)net_buf_user_data(buf);
}