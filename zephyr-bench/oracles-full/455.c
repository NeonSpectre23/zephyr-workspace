int spsc_pbuf_write(struct spsc_pbuf *pb, const char *buf, uint16_t len)
{
	char *pbuf;
	int outlen;

	if (len >= SPSC_PBUF_MAX_LEN) {
		return -EINVAL;
	}

	outlen = spsc_pbuf_alloc(pb, len, &pbuf);
	if (outlen != len) {
		return outlen < 0 ? outlen : -ENOMEM;
	}

	memcpy(pbuf, buf, len);

	spsc_pbuf_commit(pb, len);

	return len;
}