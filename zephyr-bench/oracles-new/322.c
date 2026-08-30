inline int mmc_write_blocks(struct sd_card *card, const uint8_t *wbuf, uint32_t start_block,
			    uint32_t num_blocks)
{
	return card_write_blocks(card, wbuf, start_block, num_blocks);
}