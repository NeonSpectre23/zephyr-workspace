int boot_erase_img_bank(uint8_t area_id)
{
	const struct flash_area *fa;
	int rc;

	rc = flash_area_open(area_id, &fa);
	if (rc) {
		return rc;
	}

	rc = flash_area_flatten(fa, 0, fa->fa_size);

	flash_area_close(fa);

	return rc;
}