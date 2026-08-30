int img_mgmt_client_upload(struct img_mgmt_client *client, const uint8_t *data, size_t length,
			   struct mcumgr_image_upload *res_buf)
{
	struct net_buf *nb;
	const uint8_t *write_ptr;
	int rc;
	uint32_t map_count;
	bool ok;
	size_t write_length, max_data_length, offset_before_send, request_length, wrote_length;
	zcbor_state_t zse[CONFIG_MCUMGR_SMP_CBOR_MAX_DECODING_LEVELS + 2];

	k_mutex_lock(&mcumgr_img_client_grp_mutex, K_FOREVER);
	active_client = client;
	image_upload_buf = res_buf;

	request_length = length;
	wrote_length = 0;
	/* Calculate max data length based on
	 * net_buf size - (SMP header + CBOR message_len + 16-bit CRC + 16-bit length)
	 */
	max_data_length = CONFIG_MCUMGR_TRANSPORT_NETBUF_SIZE -
			  (active_client->upload.upload_header_size + MGMT_HDR_SIZE + 2U + 2U);
	/* Trim length based on CONFIG_MCUMGR_GRP_IMG_UPLOAD_DATA_ALIGNMENT_SIZE */
	if (max_data_length % CONFIG_MCUMGR_GRP_IMG_UPLOAD_DATA_ALIGNMENT_SIZE) {
		max_data_length -=
			(max_data_length % CONFIG_MCUMGR_GRP_IMG_UPLOAD_DATA_ALIGNMENT_SIZE);
	}

	while (request_length != wrote_length) {
		write_ptr = data + wrote_length;
		write_length = request_length - wrote_length;
		if (write_length > max_data_length) {
			write_length = max_data_length;
		}

		nb = smp_client_buf_allocation(active_client->smp_client, MGMT_GROUP_ID_IMAGE,
					       IMG_MGMT_ID_UPLOAD, MGMT_OP_WRITE,
					       SMP_MCUMGR_VERSION_1);
		if (!nb) {
			image_upload_buf->status = MGMT_ERR_ENOMEM;
			goto end;
		}

		zcbor_new_encode_state(zse, ARRAY_SIZE(zse), nb->data + nb->len,
				       net_buf_tailroom(nb), 0);
		if (active_client->upload.offset) {
			map_count = 6;
		} else if (active_client->upload.hash_initialized) {
			map_count = 12;
		} else {
			map_count = 10;
		}

		/* Init map start and write image info, data and offset */
		ok = zcbor_map_start_encode(zse, map_count) && zcbor_tstr_put_lit(zse, "image") &&
		     zcbor_uint32_put(zse, active_client->upload.image_num) &&
		     zcbor_tstr_put_lit(zse, "data") &&
		     zcbor_bstr_encode_ptr(zse, write_ptr, write_length) &&
		     zcbor_tstr_put_lit(zse, "off") &&
		     zcbor_size_put(zse, active_client->upload.offset);
		/* Write Len and configured hash when offset is zero */
		if (ok && !active_client->upload.offset) {
			ok = zcbor_tstr_put_lit(zse, "len") &&
			     zcbor_size_put(zse, active_client->upload.image_size);
			if (ok && active_client->upload.hash_initialized) {
				ok = zcbor_tstr_put_lit(zse, "sha") &&
				     zcbor_bstr_encode_ptr(zse, active_client->upload.sha256,
							   IMG_MGMT_DATA_SHA_LEN);
			}
		}

		if (ok) {
			ok = zcbor_map_end_encode(zse, map_count);
		}

		if (!ok) {
			LOG_ERR("Failed to encode Image Upload packet");
			smp_packet_free(nb);
			image_upload_buf->status = MGMT_ERR_ENOMEM;
			goto end;
		}

		offset_before_send = active_client->upload.offset;
		nb->len = zse->payload - nb->data;
		k_sem_reset(&mcumgr_img_client_grp_sem);

		image_upload_buf->status = MGMT_ERR_EINVAL;
		image_upload_buf->image_upload_offset = SIZE_MAX;

		rc = smp_client_send_cmd(active_client->smp_client, nb, image_upload_res_fn,
					 &mcumgr_img_client_grp_sem,
					 CONFIG_MCUMGR_GRP_IMG_FLASH_OPERATION_TIMEOUT);
		if (rc) {
			LOG_ERR("Failed to send SMP Upload init packet, err: %d", rc);
			smp_packet_free(nb);
			image_upload_buf->status = rc;
			goto end;

		}
		k_sem_take(&mcumgr_img_client_grp_sem, K_FOREVER);
		if (image_upload_buf->status) {
			LOG_ERR("Upload Fail: %d", image_upload_buf->status);
			goto end;
		}

		if (offset_before_send + write_length < active_client->upload.offset) {
			/* Offset further than expected which indicate upload session resume */
			goto end;
		}

		wrote_length += write_length;
	}
end:
	rc = image_upload_buf->status;
	active_client = NULL;
	image_upload_buf = NULL;
	k_mutex_unlock(&mcumgr_img_client_grp_mutex);

	return rc;
}