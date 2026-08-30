int os_mgmt_client_echo(struct os_mgmt_client *client, const char *echo_string, size_t max_len)
{
	struct net_buf *nb;
	int rc;
	bool ok;
	zcbor_state_t zse[CONFIG_MCUMGR_SMP_CBOR_MAX_DECODING_LEVELS];

	k_mutex_lock(&mcummgr_os_client_grp_mutex, K_FOREVER);
	active_client = client;
	nb = smp_client_buf_allocation(active_client->smp_client, MGMT_GROUP_ID_OS, OS_MGMT_ID_ECHO,
				       MGMT_OP_WRITE, SMP_MCUMGR_VERSION_1);
	if (!nb) {
		rc = active_client->status = MGMT_ERR_ENOMEM;
		goto end;
	}

	zcbor_new_encode_state(zse, ARRAY_SIZE(zse), nb->data + nb->len, net_buf_tailroom(nb), 0);

	ok = zcbor_map_start_encode(zse, 2) &&
	     zcbor_tstr_put_lit(zse, "d") &&
	     zcbor_tstr_put_term(zse, echo_string, max_len) &&
	     zcbor_map_end_encode(zse, 2);

	if (!ok) {
		smp_packet_free(nb);
		rc = active_client->status = MGMT_ERR_ENOMEM;
		goto end;
	}

	nb->len = zse->payload - nb->data;

	LOG_DBG("Echo Command packet len %d", nb->len);
	k_sem_reset(&mcummgr_os_client_grp_sem);
	rc = smp_client_send_cmd(active_client->smp_client, nb, echo_res_fn,
				 &mcummgr_os_client_grp_sem, CONFIG_SMP_CMD_DEFAULT_LIFE_TIME);
	if (rc) {
		smp_packet_free(nb);
	} else {
		k_sem_take(&mcummgr_os_client_grp_sem, K_FOREVER);
		/* Take response status */
		rc = active_client->status;
	}
end:
	active_client = NULL;
	k_mutex_unlock(&mcummgr_os_client_grp_mutex);
	return rc;
}