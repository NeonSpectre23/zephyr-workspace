int
fcb_append_finish(struct fcb *fcb, struct fcb_entry *loc)
{
	int rc;
	uint8_t em[fcb->f_align];
	off_t off;

	(void)memset(em, 0xFF, sizeof(em));

	rc = fcb_elem_endmarker(fcb, loc, &em[0]);
	if (rc) {
		return rc;
	}
	off = loc->fe_data_off + fcb_len_in_flash(fcb, loc->fe_data_len);

	rc = fcb_flash_write(fcb, loc->fe_sector, off, em, fcb->f_align);
	if (rc) {
		return -EIO;
	}
	return 0;
}