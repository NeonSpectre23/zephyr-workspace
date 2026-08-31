int flash_area_read(const struct flash_area *fa, off_t off, void *dst,
		    size_t len)
{
	if (!is_in_flash_area_bounds(fa, off, len)) {
		return -EINVAL;
	}

	return flash_read(fa->fa_dev, fa->fa_off + off, dst, len);
}