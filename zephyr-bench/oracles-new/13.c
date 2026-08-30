int base64_decode(uint8_t *dst, size_t dlen, size_t *olen, const uint8_t *src,
		  size_t slen)
{
	size_t i, n;
	uint32_t j, x;
	uint8_t *p;

	/* First pass: check for validity and get output length */
	for (i = n = j = 0U; i < slen; i++) {
		/* Skip spaces before checking for EOL */
		x = 0U;
		while (i < slen && src[i] == ' ') {
			++i;
			++x;
		}

		/* Spaces at end of buffer are OK */
		if (i == slen) {
			break;
		}

		if ((slen - i) >= 2 && src[i] == '\r' && src[i + 1] == '\n') {
			continue;
		}

		if (src[i] == '\n') {
			continue;
		}

		/* Space inside a line is an error */
		if (x != 0U) {
			return -EINVAL;
		}

		if (src[i] == '=' && ++j > 2) {
			return -EINVAL;
		}

		if (src[i] > 127 || base64_dec_map[src[i]] == 127U) {
			return -EINVAL;
		}

		if (base64_dec_map[src[i]] < 64 && j != 0U) {
			return -EINVAL;
		}

		n++;
	}

	if (n == 0) {
		*olen = 0;
		return 0;
	}

	/* The following expression is to calculate the following formula
	 * without risk of integer overflow in n:
	 *	   n = ( ( n * 6 ) + 7 ) >> 3;
	 */
	n = (6 * (n >> 3)) + ((6 * (n & 0x7) + 7) >> 3);
	n -= j;

	if (dst == NULL || dlen < n) {
		*olen = n;
		return -ENOMEM;
	}

	for (j = 3U, n = x = 0U, p = dst; i > 0; i--, src++) {

		if (*src == '\r' || *src == '\n' || *src == ' ') {
			continue;
		}

		j -= (base64_dec_map[*src] == 64U);
		x  = (x << 6) | (base64_dec_map[*src] & 0x3F);

		if (++n == 4) {
			n = 0;
			if (j > 0) {
				*p++ = (unsigned char)(x >> 16);
			}
			if (j > 1) {
				*p++ = (unsigned char)(x >> 8);
			}
			if (j > 2) {
				*p++ = (unsigned char)(x);
			}
		}
	}

	*olen = p - dst;

	return 0;
}