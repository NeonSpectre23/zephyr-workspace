int
fcb_append_to_scratch(struct fcb *fcb)
{
	struct flash_sector *sector;
	int rc;

	sector = fcb_new_sector(fcb, 0);
	if (!sector) {
		return -ENOSPC;
	}
	rc = fcb_sector_hdr_init(fcb, sector, fcb->f_active_id + 1);
	if (rc) {
		return rc;
	}
	fcb->f_active.fe_sector = sector;
	fcb->f_active.fe_elem_off = fcb_len_in_flash(fcb, sizeof(struct fcb_disk_area));
	fcb->f_active_id++;
	return 0;
}