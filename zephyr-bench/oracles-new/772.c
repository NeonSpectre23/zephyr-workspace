int smp_dummy_tx_pkt(const uint8_t *data, int len)
{
	return mcumgr_dummy_tx_pkt(data, len, smp_receive);
}