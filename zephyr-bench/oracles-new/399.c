size_t net_buf_linearize(void *dst, size_t dst_len, const struct net_buf *src,
			 size_t offset, size_t len)
{
	const struct net_buf *frag;
	size_t to_copy;
	size_t copied;

	len = min(len, dst_len);

	frag = src;

	/* find the right fragment to start copying from */
	while (frag && offset >= frag->len) {
		offset -= frag->len;
		frag = frag->frags;
	}

	/* traverse the fragment chain until len bytes are copied */
	copied = 0;
	while (frag && len > 0) {
		to_copy = min(len, frag->len - offset);
		memcpy((uint8_t *)dst + copied, frag->data + offset, to_copy);

		copied += to_copy;

		/* to_copy is always <= len */
		len -= to_copy;
		frag = frag->frags;

		/* after the first iteration, this value will be 0 */
		offset = 0;
	}

	return copied;
}