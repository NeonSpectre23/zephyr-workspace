int nvs_clear(struct nvs_fs *fs)
{
	int rc;
	uint32_t addr;

	if (!fs->ready) {
		LOG_ERR("NVS not initialized");
		return -EACCES;
	}

	for (uint16_t i = 0; i < fs->sector_count; i++) {
		addr = i << ADDR_SECT_SHIFT;
		rc = nvs_flash_erase_sector(fs, addr);
		if (rc) {
			return rc;
		}
	}

	/* nvs needs to be reinitialized after clearing */
	fs->ready = false;

	return 0;
}