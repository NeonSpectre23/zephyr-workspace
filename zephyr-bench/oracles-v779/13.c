int cobs_encoder_write(struct cobs_encoder *enc, const uint8_t *buf, size_t len)
{
	int ret;

	__ASSERT_NO_MSG(enc != NULL);
	__ASSERT_NO_MSG(len <= INT_MAX);

	for (size_t i = 0; i < len; ++i) {
		/* Finish if group is full */
		if (enc->fragment[0] == 0xff) {
			ret = cobs_encoder_finish(enc, false);
			if (ret < 0) {
				return ret;
			}
		}

		if (buf[i] == 0x00) {
			ret = cobs_encoder_finish(enc, false);
			if (ret < 0) {
				return ret;
			}

			continue;
		}

		enc->fragment[enc->fragment[0]] = buf[i];
		enc->fragment[0]++;
	}

	return len;
}