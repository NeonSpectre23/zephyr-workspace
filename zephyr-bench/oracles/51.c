int flash_area_flatten(const struct flash_area *fa, off_t off, size_t len)
{
	if (!is_in_flash_area_bounds(fa, off, len)) {
		return -EINVAL;
	}

	return flash_flatten(fa->fa_dev, fa->fa_off + off, len);
}