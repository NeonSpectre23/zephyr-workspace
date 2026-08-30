inline int mmc_ioctl(struct sd_card *card, uint8_t cmd, void *buf)
{
	return card_ioctl(card, cmd, buf);
}