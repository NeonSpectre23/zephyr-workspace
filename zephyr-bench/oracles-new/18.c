int bindesc_open_ram(struct bindesc_handle *handle, const uint8_t *address, size_t max_size)
{
	if (!IS_ALIGNED(address, BINDESC_ALIGNMENT)) {
		LOG_ERR("Given address is not aligned");
		return -EINVAL;
	}

	if (*(uint64_t *)address != BINDESC_MAGIC) {
		LOG_ERR("Magic not found in given address");
		return -ENOENT;
	}

	handle->address = address;
	handle->type = BINDESC_HANDLE_TYPE_RAM;
	handle->size_limit = max_size;
	return 0;
}