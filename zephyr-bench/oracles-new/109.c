int fcb_free_sector_cnt(struct fcb *fcbp)
{
	int i;
	struct flash_sector *fa;

	fa = fcbp->f_active.fe_sector;
	for (i = 0; i < fcbp->f_sector_cnt; i++) {
		fa = fcb_getnext_sector(fcbp, fa);
		if (fa == fcbp->f_oldest) {
			break;
		}
	}
	return i;
}