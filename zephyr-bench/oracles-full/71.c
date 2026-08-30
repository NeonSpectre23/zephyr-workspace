int flash_area_get_sectors(int idx, uint32_t *cnt, struct flash_sector *ret)
{
	const struct flash_area *fa;
	int rc = flash_area_open(idx, &fa);

	if (rc < 0 || fa == NULL) {
		return -EINVAL;
	}

	rc = flash_area_sectors(fa, cnt, ret);
	flash_area_close(fa);

	return rc;
}