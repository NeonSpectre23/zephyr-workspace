int sd_init(const struct device *sdhc_dev, struct sd_card *card)
{
	int ret;

	if (!sdhc_dev) {
		return -ENODEV;
	}
	card->sdhc = sdhc_dev;
	ret = sdhc_get_host_props(card->sdhc, &card->host_props);
	if (ret) {
		LOG_ERR("SD host controller returned invalid properties");
		return ret;
	}

	/* init and lock card mutex */
	ret = k_mutex_init(&card->lock);
	if (ret) {
		LOG_DBG("Could not init card mutex");
		return ret;
	}
	ret = k_mutex_lock(&card->lock, K_MSEC(CONFIG_SD_INIT_TIMEOUT));
	if (ret) {
		LOG_ERR("Timeout while trying to acquire card mutex");
		return ret;
	}

	/* Initialize SDHC IO with defaults */
	ret = sd_init_io(card);
	if (ret) {
		k_mutex_unlock(&card->lock);
		return ret;
	}

	/*
	 * SD protocol is stateful, so we must account for the possibility
	 * that the card is in a bad state. The return code SD_RESTART
	 * indicates that the initialization left the card in a bad state.
	 * In this case the subsystem takes the following steps:
	 * - set card status to error
	 * - re init host I/O (will also toggle power to the SD card)
	 * - retry initialization once more
	 * If initialization then fails, the sd_init routine will assume the
	 * card is inaccessible
	 */
	ret = sd_command_init(card);
	if (ret == SD_RESTART) {
		/* Reset I/O, and retry sd initialization once more */
		card->status = CARD_ERROR;
		/* Reset I/O to default */
		ret = sd_init_io(card);
		if (ret) {
			LOG_ERR("Failed to reset SDHC I/O");
			k_mutex_unlock(&card->lock);
			return ret;
		}
		ret = sd_command_init(card);
		if (ret) {
			LOG_ERR("Failed to init SD card after I/O reset");
			k_mutex_unlock(&card->lock);
			return ret;
		}
	} else if (ret != 0) {
		/* Initialization failed */
		k_mutex_unlock(&card->lock);
		card->status = CARD_ERROR;
		return ret;
	}
	/* Card initialization succeeded. */
	card->status = CARD_INITIALIZED;
	/* Unlock card mutex */
	ret = k_mutex_unlock(&card->lock);
	if (ret) {
		LOG_DBG("Could not unlock card mutex");
		return ret;
	}
	return ret;
}