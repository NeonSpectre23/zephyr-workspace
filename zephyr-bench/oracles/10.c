int cobs_encode(struct net_buf *src, struct net_buf *dst, uint32_t flags)
{
	struct cobs_encoder enc;
	size_t len = src->len;
	int ret;

	(void)cobs_encoder_init(&enc, cobs_net_buf_cb, dst, flags);

	ret = cobs_encoder_write(&enc, net_buf_pull_mem(src, len), len);
	if (ret < 0) {
		return ret;
	}

	return cobs_encoder_close(&enc);
}