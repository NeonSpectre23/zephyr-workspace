const struct device *flash_area_get_device(const struct flash_area *fa)
{
	return fa->fa_dev;
}