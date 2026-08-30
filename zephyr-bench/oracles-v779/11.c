int cobs_encoder_close(struct cobs_encoder *enc)
{
	__ASSERT_NO_MSG(enc != NULL);

	return cobs_encoder_finish(enc, true);
}