void *smp_reassembly_get_ud(const struct smp_transport *smpt)
{
	if (smpt->__reassembly.current != NULL) {
		return net_buf_user_data(smpt->__reassembly.current);
	}

	return NULL;
}