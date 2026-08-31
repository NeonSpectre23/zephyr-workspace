int flash_area_write(const struct flash_area *fa, off_t off, const void *src,
		     size_t len)
{
	if (!is_in_flash_area_bounds(fa, off, len)) {
		return -EINVAL;
	}

	return flash_write(fa->fa_dev, fa->fa_off + off, (void *)src, len);
}