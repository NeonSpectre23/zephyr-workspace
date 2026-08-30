int smp_client_object_init(struct smp_client_object *smp_client, int smp_type)
{
	smp_client->smpt = smp_client_transport_get(smp_type);
	if (!smp_client->smpt) {
		return MGMT_ERR_EINVAL;
	}

	/* Init TX FIFO */
	k_work_init(&smp_client->work, smp_client_handle_reqs);
	k_fifo_init(&smp_client->tx_fifo);

	return MGMT_ERR_EOK;
}