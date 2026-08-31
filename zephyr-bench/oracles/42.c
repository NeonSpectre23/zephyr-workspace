int fcb_get_len(const struct fcb *fcbp, uint8_t *buf, uint16_t *len)
{
	int rc;
	uint8_t buf0_xor;
	uint8_t buf1_xor;

	buf0_xor = buf[0] ^ ~fcbp->f_erase_value;
	if (buf0_xor & 0x80) {
		if ((buf[0] == fcbp->f_erase_value) && (buf[1] == fcbp->f_erase_value)) {
			return -ENOTSUP;
		}

		buf1_xor = buf[1] ^ ~fcbp->f_erase_value;
		*len = (uint16_t)((buf0_xor & 0x7f) | ((uint16_t)buf1_xor << 7));
		rc = 2;
	} else {
		*len = (uint16_t)(buf0_xor);
		rc = 1;
	}
	return rc;
}