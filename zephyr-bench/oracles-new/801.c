void
stats_reset(struct stats_hdr *hdr)
{
	(void)memset((uint8_t *)hdr + sizeof(*hdr), 0, hdr->s_size * hdr->s_cnt);
}