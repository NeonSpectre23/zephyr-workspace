int flash_img_init_id(struct flash_img_context *ctx, uint8_t area_id)
{
	int rc;
	const struct device *flash_dev;
#if defined(CONFIG_MCUBOOT_BOOTLOADER_MODE_SWAP_USING_OFFSET)
	uint32_t sector_count = SWAP_USING_OFFSET_SECTOR_UPDATE_BEGIN;
	struct flash_sector sector_data;
#endif

	rc = flash_area_open(area_id,
			       (const struct flash_area **)&(ctx->flash_area));
	if (rc) {
		return rc;
	}

	flash_dev = flash_area_get_device(ctx->flash_area);

#if defined(CONFIG_MCUBOOT_BOOTLOADER_MODE_SWAP_USING_OFFSET)
	/* Query size of first sector in flash for upgrade slot, so it can be erased, and begin
	 * upload started at the second sector
	 */
	rc = flash_area_sectors((const struct flash_area *)ctx->flash_area, &sector_count,
				&sector_data);

	if (rc && rc != -ENOMEM) {
		flash_area_close(ctx->flash_area);
		ctx->flash_area = NULL;
		return rc;
	} else if (sector_count != SWAP_USING_OFFSET_SECTOR_UPDATE_BEGIN) {
		flash_area_close(ctx->flash_area);
		ctx->flash_area = NULL;
		return -ENOENT;
	}

	if (!flash_check_erased((const struct flash_area *)ctx->flash_area)) {
		/* Flash is not empty, therefore flatten/erase the area to prevent issues when
		 * the firmware update process begins
		 */
		rc = flash_area_flatten((const struct flash_area *)ctx->flash_area, 0,
					sector_data.fs_size);

		if (rc) {
			flash_area_close(ctx->flash_area);
			ctx->flash_area = NULL;
			return rc;
		}
	}

	return stream_flash_init(&ctx->stream, flash_dev, ctx->buf, CONFIG_IMG_BLOCK_BUF_SIZE,
				 (ctx->flash_area->fa_off + sector_data.fs_size),
				 (ctx->flash_area->fa_size - sector_data.fs_size), NULL);
#else
	return stream_flash_init(&ctx->stream, flash_dev, ctx->buf,
			CONFIG_IMG_BLOCK_BUF_SIZE, ctx->flash_area->fa_off,
			ctx->flash_area->fa_size, NULL);
#endif
}