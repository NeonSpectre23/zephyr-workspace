int nvmem_cell_write(const struct nvmem_cell *cell, const void *buf, off_t off, size_t len)
{
	__ASSERT_NO_MSG(cell != NULL);

	if (off < 0 || cell->size < off + len) {
		return -EINVAL;
	}

	if (cell->read_only) {
		return -EROFS;
	}

	if (!device_is_ready(cell->dev)) {
		return -ENODEV;
	}

	if (IS_ENABLED(CONFIG_NVMEM_BBRAM) && DEVICE_API_IS(bbram, cell->dev)) {
		return bbram_write(cell->dev, cell->offset + off, len, buf);
	}

	if (IS_ENABLED(CONFIG_NVMEM_EEPROM) && DEVICE_API_IS(eeprom, cell->dev)) {
		return eeprom_write(cell->dev, cell->offset + off, buf, len);
	}

	if (IS_ENABLED(CONFIG_NVMEM_FLASH_WRITE) && DEVICE_API_IS(flash, cell->dev)) {
		return flash_write(cell->dev, cell->offset + off, buf, len);
	}

	if (IS_ENABLED(CONFIG_NVMEM_OTP_WRITE) && DEVICE_API_IS(otp, cell->dev)) {
		return otp_program(cell->dev, cell->offset + off, buf, len);
	}

	return -ENXIO;
}