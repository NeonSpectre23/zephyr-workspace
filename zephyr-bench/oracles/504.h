static inline int fcb_len_in_flash(struct fcb *fcbp, uint16_t len)
{
	if (fcbp->f_align <= 1U) {
		return len;
	}
	return (len + (fcbp->f_align - 1U)) & ~(fcbp->f_align - 1U);
}