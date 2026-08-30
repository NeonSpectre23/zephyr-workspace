int bindesc_open_flash(struct bindesc_handle *handle, size_t offset,
		       const struct device *flash_device)
{
	int retval;

	retval = flash_read(flash_device, offset, handle->buffer, sizeof(BINDESC_MAGIC));
	if (retval) {
		LOG_ERR("Flash read error: %d", retval);
		return -EIO;
	}

	if (*(uint64_t *)handle->buffer != BINDESC_MAGIC) {
		LOG_ERR("Magic not found in given address");
		return -ENOENT;
	}

	handle->address = (uint8_t *)offset;
	handle->type = BINDESC_HANDLE_TYPE_FLASH;
	handle->flash_device = flash_device;
	handle->size_limit = UINT16_MAX;
	return 0;
}