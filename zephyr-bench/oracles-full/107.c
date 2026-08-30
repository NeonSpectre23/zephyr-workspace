int isotp_recv(struct isotp_recv_ctx *rctx, uint8_t *data, size_t len, k_timeout_t timeout)
{
	size_t copied, to_copy;
	int err;

	if (!rctx->recv_buf) {
		rctx->recv_buf = k_fifo_get(&rctx->fifo, timeout);
		if (!rctx->recv_buf) {
			err = rctx->error_nr ? rctx->error_nr : ISOTP_RECV_TIMEOUT;
			rctx->error_nr = 0;

			return err;
		}
	}

	/* traverse fragments and delete them after copying the data */
	copied = 0;
	while (rctx->recv_buf && copied < len) {
		to_copy = MIN(len - copied, rctx->recv_buf->len);
		memcpy((uint8_t *)data + copied, rctx->recv_buf->data, to_copy);

		if (rctx->recv_buf->len == to_copy) {
			/* point recv_buf to next frag */
			rctx->recv_buf = net_buf_frag_del(NULL, rctx->recv_buf);
		} else {
			/* pull received data from remaining frag(s) */
			net_buf_pull(rctx->recv_buf, to_copy);
		}

		copied += to_copy;
	}

	return copied;
}