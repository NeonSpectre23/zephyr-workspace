int boot_write_img_confirmed(void)
{
	const struct flash_area *fa;
	int rc = 0;

	if (flash_area_open(ACTIVE_SLOT_FLASH_AREA_ID, &fa) != 0) {
		return -EIO;
	}

	rc = boot_set_next(fa, true, true);

	flash_area_close(fa);

	return rc;
}