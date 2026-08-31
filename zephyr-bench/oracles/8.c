int cobs_decoder_init(struct cobs_decoder *dec, cobs_stream_cb cb, void *user_data, uint32_t flags)
{
	if (cb == NULL) {
		return -EINVAL;
	}

	__ASSERT_NO_MSG(dec != NULL);

	dec->cb = cb;
	dec->cb_user_data = user_data;
	dec->flags = flags;

	cobs_decoder_reset(dec);

	return 0;
}