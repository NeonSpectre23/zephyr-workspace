int fcb_init(int f_area_id, struct fcb *fcbp)
{
	struct flash_sector *sector;
	int rc;
	int i;
	uint8_t align;
	int oldest = -1, newest = -1;
	struct flash_sector *oldest_sector = NULL, *newest_sector = NULL;
	struct fcb_disk_area fda;
	const struct flash_parameters *fparam;

	if (!fcbp->f_sectors || fcbp->f_sector_cnt - fcbp->f_scratch_cnt < 1) {
		return -EINVAL;
	}

	rc = flash_area_open(f_area_id, &fcbp->fap);
	if (rc != 0) {
		return -EINVAL;
	}

	fparam = flash_get_parameters(fcbp->fap->fa_dev);
	fcbp->f_erase_value = fparam->erase_value;

	align = fcb_get_align(fcbp);
	if (align == 0U) {
		return -EINVAL;
	}

	/* Fill last used, first used */
	for (i = 0; i < fcbp->f_sector_cnt; i++) {
		sector = &fcbp->f_sectors[i];
		rc = fcb_sector_hdr_read(fcbp, sector, &fda);
		if (rc < 0) {
			return rc;
		}
		if (rc == 0) {
			continue;
		}
		if (oldest < 0) {
			oldest = newest = fda.fd_id;
			oldest_sector = newest_sector = sector;
			continue;
		}
		if (FCB_ID_GT(fda.fd_id, newest)) {
			newest = fda.fd_id;
			newest_sector = sector;
		} else if (FCB_ID_GT(oldest, fda.fd_id)) {
			oldest = fda.fd_id;
			oldest_sector = sector;
		}
	}
	if (oldest < 0) {
		/*
		 * No initialized areas.
		 */
		oldest_sector = newest_sector = &fcbp->f_sectors[0];
		rc = fcb_sector_hdr_init(fcbp, oldest_sector, 0);
		if (rc) {
			return rc;
		}
		newest = oldest = 0;
	}
	fcbp->f_align = align;
	fcbp->f_oldest = oldest_sector;
	fcbp->f_active.fe_sector = newest_sector;
	fcbp->f_active.fe_elem_off = fcb_len_in_flash(fcbp, sizeof(struct fcb_disk_area));
	fcbp->f_active_id = newest;

	while (1) {
		rc = fcb_getnext_in_sector(fcbp, &fcbp->f_active);
		if (rc == -ENOTSUP) {
			rc = 0;
			break;
		}
		if (rc != 0) {
			break;
		}
	}
	k_mutex_init(&fcbp->f_mtx);
	return rc;
}