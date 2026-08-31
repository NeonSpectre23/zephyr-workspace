int
fcb_append(struct fcb *fcb, uint16_t len, struct fcb_entry *append_loc)
{
	struct flash_sector *sector;
	struct fcb_entry *active;
	int cnt;
	int rc;
	uint8_t tmp_str[MAX(8, fcb->f_align)];

	/* Ensure defined value of padding bytes */
	memset(tmp_str, fcb->f_erase_value, sizeof(tmp_str));

	cnt = fcb_put_len(fcb, tmp_str, len);
	if (cnt < 0) {
		return cnt;
	}
	cnt = fcb_len_in_flash(fcb, cnt);
	len = fcb_len_in_flash(fcb, len) + fcb_len_in_flash(fcb, FCB_CRC_SZ);

	__ASSERT_NO_MSG(cnt <= sizeof(tmp_str));

	rc = k_mutex_lock(&fcb->f_mtx, K_FOREVER);
	if (rc) {
		return -EINVAL;
	}
	active = &fcb->f_active;
	if (active->fe_elem_off + len + cnt > active->fe_sector->fs_size) {
		sector = fcb_new_sector(fcb, fcb->f_scratch_cnt);
		if (!sector || (sector->fs_size <
			fcb_len_in_flash(fcb, sizeof(struct fcb_disk_area)) + len + cnt)) {
			rc = -ENOSPC;
			goto err;
		}
		rc = fcb_sector_hdr_init(fcb, sector, fcb->f_active_id + 1);
		if (rc) {
			goto err;
		}
		fcb->f_active.fe_sector = sector;
		fcb->f_active.fe_elem_off = fcb_len_in_flash(fcb, sizeof(struct fcb_disk_area));
		fcb->f_active_id++;
	}

	rc = fcb_flash_write(fcb, active->fe_sector, active->fe_elem_off, tmp_str, cnt);
	if (rc) {
		rc = -EIO;
		goto err;
	}
	append_loc->fe_sector = active->fe_sector;
	append_loc->fe_elem_off = active->fe_elem_off;
	append_loc->fe_data_off = active->fe_elem_off + cnt;

	active->fe_elem_off = append_loc->fe_data_off + len;

	k_mutex_unlock(&fcb->f_mtx);

	return 0;
err:
	k_mutex_unlock(&fcb->f_mtx);
	return rc;
}