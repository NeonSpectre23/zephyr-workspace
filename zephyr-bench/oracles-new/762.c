struct net_buf *smp_client_buf_allocation(struct smp_client_object *smp_client, uint16_t group,
					  uint8_t command_id, uint8_t op,
					  enum smp_mcumgr_version_t version)
{
	struct net_buf *nb;
	struct smp_hdr smp_header;

	nb = smp_alloc_req(smp_client->smpt, smp_client_object_get_data(smp_client));

	if (nb) {
		/* Write SMP header with payload length 0 */
		smp_header_init(&smp_header, group, command_id, op, 0, smp_client->smp_seq++,
				version);
		memcpy(nb->data, &smp_header, sizeof(smp_header));
		nb->len = sizeof(smp_header);
	}
	return nb;
}