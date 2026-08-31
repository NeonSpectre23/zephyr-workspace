int cobs_encoder_init(struct cobs_encoder *enc, cobs_stream_cb cb, void *user_data, uint32_t flags)
{
	if (cb == NULL) {
		return -EINVAL;
	}

	__ASSERT_NO_MSG(enc != NULL);

	enc->cb = cb;
	enc->cb_user_data = user_data;
	enc->flags = flags;

	cobs_encoder_reset(enc);

	return 0;
}