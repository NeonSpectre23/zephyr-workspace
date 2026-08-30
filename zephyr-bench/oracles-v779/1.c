size_t bin2hex(const uint8_t *buf, size_t buflen, char *hex, size_t hexlen)
{
	if (hexlen < ((buflen * 2U) + 1U)) {
		return 0;
	}

	for (size_t i = 0; i < buflen; i++) {
		hex2char(buf[i] >> 4, &hex[2U * i]);
		hex2char(buf[i] & 0xf, &hex[2U * i + 1U]);
	}

	hex[2U * buflen] = '\0';
	return 2U * buflen;
}