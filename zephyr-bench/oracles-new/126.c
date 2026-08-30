int flash_area_open(uint8_t id, const struct flash_area **fap)
{
	const struct flash_area *area;

	if (flash_map == NULL) {
		return -EACCES;
	}

	area = get_flash_area_from_id(id);
	if (area == NULL) {
		return -ENOENT;
	}

	if (!device_is_ready(area->fa_dev)) {
		return -ENODEV;
	}

	*fap = area;

	return 0;
}