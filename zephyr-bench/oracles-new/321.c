inline int mmc_read_blocks(struct sd_card *card, uint8_t *rbuf, uint32_t start_block,
			   uint32_t num_blocks)
{
	return card_read_blocks(card, rbuf, start_block, num_blocks);
}