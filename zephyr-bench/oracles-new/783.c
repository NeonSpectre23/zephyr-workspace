int smp_reassembly_complete(struct smp_transport *smpt, bool force)
{
	if (smpt->__reassembly.current == NULL) {
		return -EINVAL;
	}

	if (smpt->__reassembly.expected == 0 || force) {
		int expected = smpt->__reassembly.expected;

		smp_rx_req(smpt, smpt->__reassembly.current);
		smpt->__reassembly.expected = 0;
		smpt->__reassembly.current = NULL;
		return expected;
	}
	return -ENODATA;
}