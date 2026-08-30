int nvs_mount(struct nvs_fs *fs)
{
	int rc;
	struct flash_pages_info info;
	size_t write_block_size;

	k_mutex_init(&fs->nvs_lock);

	fs->flash_parameters = flash_get_parameters(fs->flash_device);
	if (fs->flash_parameters == NULL) {
		LOG_ERR("Could not obtain flash parameters");
		return -EINVAL;
	}

	write_block_size = flash_get_write_block_size(fs->flash_device);

	/* check that the write block size is supported */
	if (write_block_size > NVS_BLOCK_SIZE || write_block_size == 0) {
		LOG_ERR("Unsupported write block size");
		return -EINVAL;
	}

	/* check that sector size is not greater than max */
	if (fs->sector_size > NVS_MAX_SECTOR_SIZE) {
		LOG_ERR("Sector size %u too large, maximum is %zu",
			fs->sector_size, NVS_MAX_SECTOR_SIZE);
		return -EINVAL;
	}

	/* check that sector size is a multiple of pagesize */
	rc = flash_get_page_info_by_offs(fs->flash_device, fs->offset, &info);
	if (rc) {
		LOG_ERR("Unable to get page info");
		return -EINVAL;
	}
	if (!fs->sector_size || fs->sector_size % info.size) {
		LOG_ERR("Invalid sector size");
		return -EINVAL;
	}

	/* check the number of sectors, it should be at least 2 */
	if (fs->sector_count < 2) {
		LOG_ERR("Configuration error - sector count");
		return -EINVAL;
	}

	rc = nvs_startup(fs);
	if (rc) {
		return rc;
	}

	/* nvs is ready for use */
	fs->ready = true;

	LOG_INF("%d Sectors of %d bytes", fs->sector_count, fs->sector_size);
	LOG_INF("alloc wra: %d, %x",
		(fs->ate_wra >> ADDR_SECT_SHIFT),
		(fs->ate_wra & ADDR_OFFS_MASK));
	LOG_INF("data wra: %d, %x",
		(fs->data_wra >> ADDR_SECT_SHIFT),
		(fs->data_wra & ADDR_OFFS_MASK));

	return 0;
}