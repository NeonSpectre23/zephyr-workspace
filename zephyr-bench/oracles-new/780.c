int smp_raw_dummy_tx_pkt(const uint8_t *data, int len)
{
	uint16_t data_size =
		MIN(len, (sizeof(smp_receive_buffer) - smp_receive_pos - 1));

	if (enable_dummy_smp == true) {
		memcpy(&smp_receive_buffer[smp_receive_pos], data, data_size);
		smp_receive_pos += data_size;
	}

	return 0;
}