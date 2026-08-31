int cobs_decode(struct net_buf *src, struct net_buf *dst, uint32_t flags)
{
	struct cobs_decoder dec;
	size_t len = src->len;
	int ret;

	(void)cobs_decoder_init(&dec, cobs_net_buf_cb, dst, flags);

	ret = cobs_decoder_write(&dec, net_buf_pull_mem(src, len), len);
	if (ret < 0) {
		return ret;
	}

	return cobs_decoder_close(&dec);
}