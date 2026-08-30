uint32_t flash_area_align(const struct flash_area *fa)
{
	return flash_get_write_block_size(fa->fa_dev);
}