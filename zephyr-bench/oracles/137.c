int mipi_stp_decoder_init(const struct mipi_stp_decoder_config *config)
{
	state = config->start_out_of_sync ? STP_STATE_OUT_OF_SYNC : STP_STATE_OP;
	ntotal = 0;
	ncnt = 0;
	cfg = *config;
	prev_ts = 0;
	base_ts = 0;
	noff = 0;

	return 0;
}