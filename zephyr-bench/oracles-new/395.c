size_t net_buf_data_match(const struct net_buf *buf, size_t offset, const void *data, size_t len)
{
	const uint8_t *dptr = data;
	const uint8_t *bptr;
	size_t compared = 0;
	size_t to_compare;

	if (!buf || !data) {
		return compared;
	}

	/* find the right fragment to start comparison */
	while (buf && offset >= buf->len) {
		offset -= buf->len;
		buf = buf->frags;
	}

	while (buf && len > 0) {
		bptr = buf->data + offset;
		to_compare = min(len, buf->len - offset);

		for (size_t i = 0; i < to_compare; ++i) {
			if (dptr[compared] != bptr[i]) {
				return compared;
			}
			compared++;
		}

		len -= to_compare;
		buf = buf->frags;
		offset = 0;
	}

	return compared;
}