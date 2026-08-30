int img_mgmt_client_state_read(struct img_mgmt_client *client, struct mcumgr_image_state *res_buf)
{
	struct net_buf *nb;
	int rc;
	zcbor_state_t zse[CONFIG_MCUMGR_SMP_CBOR_MAX_DECODING_LEVELS];
	bool ok;

	k_mutex_lock(&mcumgr_img_client_grp_mutex, K_FOREVER);
	active_client = client;
	/* Init Response */
	res_buf->image_list_length = 0;
	res_buf->image_list = active_client->image_list;

	image_info = res_buf;

	nb = smp_client_buf_allocation(active_client->smp_client, MGMT_GROUP_ID_IMAGE,
				       IMG_MGMT_ID_STATE, MGMT_OP_READ, SMP_MCUMGR_VERSION_1);
	if (!nb) {
		res_buf->status = MGMT_ERR_ENOMEM;
		goto end;
	}

	zcbor_new_encode_state(zse, ARRAY_SIZE(zse), nb->data + nb->len, net_buf_tailroom(nb), 0);
	ok = zcbor_map_start_encode(zse, 1) && zcbor_map_end_encode(zse, 1);
	if (!ok) {
		smp_packet_free(nb);
		res_buf->status = MGMT_ERR_ENOMEM;
		goto end;
	}

	nb->len = zse->payload - nb->data;
	k_sem_reset(&mcumgr_img_client_grp_sem);
	rc = smp_client_send_cmd(active_client->smp_client, nb, image_state_res_fn,
				 &mcumgr_img_client_grp_sem, CONFIG_SMP_CMD_DEFAULT_LIFE_TIME);
	if (rc) {
		smp_packet_free(nb);
		res_buf->status = rc;
		goto end;
	}
	k_sem_take(&mcumgr_img_client_grp_sem, K_FOREVER);
end:
	rc = res_buf->status;
	image_info = NULL;
	active_client = NULL;
	k_mutex_unlock(&mcumgr_img_client_grp_mutex);
	return rc;
}