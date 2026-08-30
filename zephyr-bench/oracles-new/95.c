static int data_read(off_t off, uint8_t *dst, size_t len,
		     data_read_cb_t cb, void *cb_arg)
{
	int ret = 0;
	off_t offset = off;
	size_t remaining = len;
	size_t copy_sz;
	uint8_t *ptr = dst;

	if (backend_ctx.flash_area == NULL) {
		return -ENODEV;
	}

	copy_sz = FLASH_BUF_SIZE;
	while (remaining > 0) {
		if (remaining < FLASH_BUF_SIZE) {
			copy_sz = remaining;
		}

		ret = flash_area_read(backend_ctx.flash_area, offset,
				      data_read_buf, FLASH_BUF_SIZE);
		if (ret != 0) {
			break;
		}

		if (dst != NULL) {
			(void)memcpy(ptr, data_read_buf, copy_sz);
		}

		if (cb != NULL) {
			ret = (*cb)(cb_arg, data_read_buf, copy_sz);
			if (ret != 0) {
				break;
			}
		}

		ptr += copy_sz;
		offset += copy_sz;
		remaining -= copy_sz;
	}

	if (cb != NULL) {
		ret = (*cb)(cb_arg, NULL, 0);
	}

	return ret;
}