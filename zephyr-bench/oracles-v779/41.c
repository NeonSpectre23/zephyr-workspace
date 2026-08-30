int fcb_elem_info(struct fcb *_fcb, struct fcb_entry *loc)
{
	int rc;
	uint8_t em;
	uint8_t fl_em;
	off_t off;

	rc = fcb_elem_endmarker(_fcb, loc, &em);
	if (rc) {
		return rc;
	}
	off = loc->fe_data_off + fcb_len_in_flash(_fcb, loc->fe_data_len);

	rc = fcb_flash_read(_fcb, loc->fe_sector, off, &fl_em, sizeof(fl_em));
	if (rc) {
		return -EIO;
	}

	if (IS_ENABLED(CONFIG_FCB_ALLOW_FIXED_ENDMARKER) && (fl_em != em)) {
		rc = fcb_elem_crc8(_fcb, loc, &em);
		if (rc) {
			return rc;
		}
	}

	if (fl_em != em) {
		return -EBADMSG;
	}
	return 0;
}