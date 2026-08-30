int sdio_read_byte(struct sdio_func *func, uint32_t reg, uint8_t *val)
{
	int ret;

	if ((func->card->type != CARD_SDIO) && (func->card->type != CARD_COMBO)) {
		LOG_WRN("Card does not support SDIO commands");
		return -ENOTSUP;
	}
	ret = k_mutex_lock(&func->card->lock, K_MSEC(CONFIG_SD_DATA_TIMEOUT));
	if (ret) {
		LOG_WRN("Could not get SD card mutex");
		return -EBUSY;
	}
	ret = sdio_io_rw_direct(func->card, SDIO_IO_READ, func->num, reg, 0, val);
	k_mutex_unlock(&func->card->lock);
	return ret;
}