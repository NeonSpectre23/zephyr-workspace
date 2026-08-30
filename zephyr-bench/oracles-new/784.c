int smp_reassembly_drop(struct smp_transport *smpt)
{
	if (smpt->__reassembly.current == NULL) {
		return -EINVAL;
	}

	smp_packet_free(smpt->__reassembly.current);
	smpt->__reassembly.expected = 0;
	smpt->__reassembly.current = NULL;

	return 0;
}