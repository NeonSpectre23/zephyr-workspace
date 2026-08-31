int cobs_decoder_close(struct cobs_decoder *dec)
{
	int ret;

	__ASSERT_NO_MSG(dec != NULL);

	ret = cobs_decoder_needs_more_data(dec) ? -EINVAL : 0;
	cobs_decoder_reset(dec);

	return ret;
}