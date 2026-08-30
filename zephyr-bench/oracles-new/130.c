int flash_img_buffered_write(struct flash_img_context *ctx, const uint8_t *data,
			     size_t len, bool flush)
{
	int rc;

	/* If there is a need to erase the trailer, that should happen before any
	 * write is done to partition.
	 */
	rc = scramble_mcuboot_trailer(ctx);
	if (rc != 0) {
		return rc;
	}


	/* if CONFIG_IMG_ERASE_PROGRESSIVELY is enabled the enabled CONFIG_STREAM_FLASH_ERASE
	 * ensures that stream_flash erases flash progresively.
	 */
	rc = stream_flash_buffered_write(&ctx->stream, data, len, flush);
	if (!flush) {
		return rc;
	}

	flash_area_close(ctx->flash_area);
	ctx->flash_area = NULL;

	return rc;
}