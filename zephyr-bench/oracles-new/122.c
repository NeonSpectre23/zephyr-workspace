uint8_t flash_area_erased_val(const struct flash_area *fa)
{
	const struct flash_parameters *param;

	param = flash_get_parameters(fa->fa_dev);

	return param->erase_value;
}