int spsc_pbuf_read(struct spsc_pbuf *pb, char *buf, uint16_t len)
{
	char *pkt;
	uint16_t plen = spsc_pbuf_claim(pb, &pkt);

	if (plen == 0) {
		return 0;
	}

	if (buf == NULL) {
		return plen;
	}

	if (len < plen) {
		return -ENOMEM;
	}

	memcpy(buf, pkt, plen);

	spsc_pbuf_free(pb, plen);

	return plen;
}