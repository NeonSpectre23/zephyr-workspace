int img_mgmt_client_erase(struct img_mgmt_client *client, uint32_t slot)
{
	struct net_buf *nb;
	int rc;
	zcbor_state_t zse[CONFIG_MCUMGR_SMP_CBOR_MAX_DECODING_LEVELS];
	bool ok;

	k_mutex_lock(&mcumgr_img_client_grp_mutex, K_FOREVER);
	active_client = client;

	nb = smp_client_buf_allocation(active_client->smp_client, MGMT_GROUP_ID_IMAGE,
				       IMG_MGMT_ID_ERASE, MGMT_OP_WRITE, SMP_MCUMGR_VERSION_1);
	if (!nb) {
		active_client->status = MGMT_ERR_ENOMEM;
		goto end;
	}

	zcbor_new_encode_state(zse, ARRAY_SIZE(zse), nb->data + nb->len, net_buf_tailroom(nb), 0);

	ok = zcbor_map_start_encode(zse, 2) && zcbor_tstr_put_lit(zse, "slot") &&
	     zcbor_uint32_put(zse, slot) && zcbor_map_end_encode(zse, 2);
	if (!ok) {
		smp_packet_free(nb);
		active_client->status = MGMT_ERR_ENOMEM;
		goto end;
	}

	nb->len = zse->payload - nb->data;
	k_sem_reset(&mcumgr_img_client_grp_sem);
	rc = smp_client_send_cmd(client->smp_client, nb, erase_res_fn, &mcumgr_img_client_grp_sem,
				 CONFIG_MCUMGR_GRP_IMG_FLASH_OPERATION_TIMEOUT);
	if (rc) {
		smp_packet_free(nb);
		active_client->status = rc;
		goto end;
	}
	k_sem_take(&mcumgr_img_client_grp_sem, K_FOREVER);
end:
	rc = active_client->status;
	active_client = NULL;
	k_mutex_unlock(&mcumgr_img_client_grp_mutex);
	return rc;
}