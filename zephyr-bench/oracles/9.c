int cobs_decoder_write(struct cobs_decoder *dec, const uint8_t *buf, size_t len)
{
	uint8_t sentinel = COBS_FLAG_CUSTOM_DELIMITER(dec->flags);
	int ret;

	__ASSERT_NO_MSG(dec != NULL);
	__ASSERT_NO_MSG(len <= INT_MAX);

	for (size_t i = 0; i < len; ++i) {
		uint8_t data = buf[i] ^ sentinel;

		if (data == 0x00) {
			if ((dec->flags & COBS_FLAG_TRAILING_DELIMITER) == 0U ||
			    cobs_decoder_needs_more_data(dec)) {
				/* Decoder shouldn't get delimiters or unexpected end of data */
				cobs_decoder_reset(dec);
				return -EINVAL;
			}

			/* Notify frame delimiter was seen */
			ret = dec->cb(NULL, 0, dec->cb_user_data);
			if (ret < 0) {
				cobs_decoder_reset(dec);
				return ret;
			}

			/* Reset state */
			cobs_decoder_reset(dec);
			continue;
		}

		if (dec->code_index > 0) {
			ret = dec->cb(&data, 1, dec->cb_user_data);
			if (ret < 0) {
				cobs_decoder_reset(dec);
				return ret;
			}

			dec->code_index--;
			continue;
		}

		dec->code_index = data;

		if (dec->code != 0xff) {
			/* Group finished, output zero byte */
			data = 0x00;

			ret = dec->cb(&data, 1, dec->cb_user_data);
			if (ret < 0) {
				cobs_decoder_reset(dec);
				return ret;
			}
		}

		dec->code = dec->code_index;
		dec->code_index--;
	}

	return len;
}