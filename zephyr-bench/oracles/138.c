void mipi_stp_decoder_sync_loss(void)
{
	state = STP_STATE_OUT_OF_SYNC;
	ncnt = 0;
	ntotal = 0;
}