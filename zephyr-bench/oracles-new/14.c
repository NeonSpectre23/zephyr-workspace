int base64_encode(uint8_t *dst, size_t dlen, size_t *olen, const uint8_t *src,
		  size_t slen)
{
	size_t i, n;
	int C1, C2, C3;
	uint8_t *p;

	if (slen == 0) {
		*olen = 0;
		return 0;
	}

	n = slen / 3 + (slen % 3 != 0);

	if (n > (BASE64_SIZE_T_MAX - 1) / 4) {
		*olen = BASE64_SIZE_T_MAX;
		return -ENOMEM;
	}

	n *= 4;

	if ((dlen < n + 1) || (!dst)) {
		*olen = n + 1;
		return -ENOMEM;
	}

	n = (slen / 3) * 3;

	for (i = 0, p = dst; i < n; i += 3) {
		C1 = *src++;
		C2 = *src++;
		C3 = *src++;

		*p++ = base64_enc_map[(C1 >> 2) & 0x3F];
		*p++ = base64_enc_map[(((C1 &  3) << 4) + (C2 >> 4)) & 0x3F];
		*p++ = base64_enc_map[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
		*p++ = base64_enc_map[C3 & 0x3F];
	}

	if (i < slen) {
		C1 = *src++;
		C2 = ((i + 1) < slen) ? *src++ : 0;

		*p++ = base64_enc_map[(C1 >> 2) & 0x3F];
		*p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];

		if ((i + 1) < slen) {
			*p++ = base64_enc_map[((C2 & 15) << 2) & 0x3F];
		} else {
			*p++ = '=';
		}

		*p++ = '=';
	}

	*olen = p - dst;
	*p = 0U;

	return 0;
}