static ALWAYS_INLINE bool flash_area_device_is_ready(const struct flash_area *fa)
{
	return (fa != NULL && device_is_ready(fa->fa_dev));
}