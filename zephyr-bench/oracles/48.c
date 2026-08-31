int flash_area_copy(const struct flash_area *src_fa, off_t src_off,
		    const struct flash_area *dst_fa, off_t dst_off,
		    off_t len, uint8_t *buf, size_t buf_size)
{
	if (!(is_in_flash_area_bounds(src_fa, src_off, len) &&
	      is_in_flash_area_bounds(dst_fa, dst_off, len))) {
		return -EINVAL;
	}

	return flash_copy(src_fa->fa_dev, src_fa->fa_off + src_off,
			  dst_fa->fa_dev, dst_fa->fa_off + dst_off, len, buf,
			  buf_size);
}