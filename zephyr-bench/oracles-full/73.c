int flash_area_sectors(const struct flash_area *fa, uint32_t *cnt, struct flash_sector *ret)
{
	struct layout_data data;
	const struct device *flash_dev;

	data.area_off = fa->fa_off;
	data.area_len = fa->fa_size;

	data.ret = ret;
	data.ret_idx = 0U;
	data.ret_len = *cnt;
	data.status = 0;

	flash_dev = fa->fa_dev;

	flash_page_foreach(flash_dev, get_sectors_cb, &data);

	if (data.status == 0) {
		*cnt = data.ret_idx;
	}

	return data.status;
}