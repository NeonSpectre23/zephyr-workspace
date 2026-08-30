int img_mgmt_client_state_write(struct img_mgmt_client *client, char *hash, bool confirm,
				struct mcumgr_image_state *res_buf)
{
	struct net_buf *nb;
	int rc;
	uint32_t map_count;
	zcbor_state_t zse[CONFIG_MCUMGR_SMP_CBOR_MAX_DECODING_LEVELS];
	bool ok;

	k_mutex_lock(&mcumgr_img_client_grp_mutex, K_FOREVER);
	active_client = client;
	image_info = res_buf;
	/* Init Response */
	res_buf->image_list_length = 0;
	res_buf->image_list = active_client->image_list;

	nb = smp_client_buf_allocation(active_client->smp_client, MGMT_GROUP_ID_IMAGE,
				       IMG_MGMT_ID_STATE, MGMT_OP_WRITE, SMP_MCUMGR_VERSION_1);
	if (!nb) {
		res_buf->status = MGMT_ERR_ENOMEM;
		goto end;
	}

	zcbor_new_encode_state(zse, ARRAY_SIZE(zse), nb->data + nb->len, net_buf_tailroom(nb), 0);
	if (hash) {
		map_count = 4;
	} else {
		map_count = 2;
	}

	/* Write map start init and confirm params */
	ok = zcbor_map_start_encode(zse, map_count) && zcbor_tstr_put_lit(zse, "confirm") &&
	     zcbor_bool_put(zse, confirm);
	/* Write hash data */
	if (ok && hash) {
		ok = zcbor_tstr_put_lit(zse, "hash") &&
		     zcbor_bstr_encode_ptr(zse, hash, IMG_MGMT_DATA_SHA_LEN);
	}
	/* Close map */
	if (ok) {
		ok = zcbor_map_end_encode(zse, map_count);
	}

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
		res_buf->status = rc;
		smp_packet_free(nb);
		goto end;
	}
	k_sem_take(&mcumgr_img_client_grp_sem, K_FOREVER);
end:
	rc = res_buf->status;
	active_client = NULL;
	k_mutex_unlock(&mcumgr_img_client_grp_mutex);
	return rc;
}