int fcb_put_len(const struct fcb *fcbp, uint8_t *buf, uint16_t len)
{
	if (len < 0x80) {
		buf[0] = len ^ ~fcbp->f_erase_value;
		return 1;
	} else if (len <= FCB_MAX_LEN) {
		buf[0] = (len | 0x80) ^ ~fcbp->f_erase_value;
		buf[1] = (len >> 7) ^ ~fcbp->f_erase_value;
		return 2;
	} else {
		return -EINVAL;
	}
}