int img_mgmt_client_upload_init(struct img_mgmt_client *client, size_t image_size,
				uint32_t image_num, const char *image_hash)
{
	int rc;

	k_mutex_lock(&mcumgr_img_client_grp_mutex, K_FOREVER);
	client->upload.image_size = image_size;
	client->upload.offset = 0;
	client->upload.image_num = image_num;
	if (image_hash) {
		memcpy(client->upload.sha256, image_hash, IMG_MGMT_DATA_SHA_LEN);
		client->upload.hash_initialized = true;
	} else {
		client->upload.hash_initialized = false;
	}

	/* Calculate worst case header size for adapt payload length */
	client->upload.upload_header_size = upload_message_header_size(&client->upload);
	if (client->upload.upload_header_size) {
		rc = MGMT_ERR_EOK;
	} else {
		rc = MGMT_ERR_ENOMEM;
	}
	k_mutex_unlock(&mcumgr_img_client_grp_mutex);
	return rc;
}