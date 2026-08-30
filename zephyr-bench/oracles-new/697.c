bool sd_is_card_present(const struct device *sdhc_dev)
{
	if (!sdhc_dev) {
		return false;
	}
	return sdhc_card_present(sdhc_dev) == 1;
}