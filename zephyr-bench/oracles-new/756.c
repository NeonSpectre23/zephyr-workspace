int sip_svc_unregister(void *ct, uint32_t c_token)
{
	int err;
	uint32_t c_idx;

	if (ct == NULL || !is_sip_svc_controller(ct)) {
		return -EINVAL;
	}

	struct sip_svc_controller *ctrl = (struct sip_svc_controller *)ct;

	err = k_mutex_lock(&ctrl->data_mutex, K_FOREVER);
	if (err != 0) {
		LOG_ERR("Error in acquiring mutex %d", err);
		return -ENOLCK;
	}

	c_idx = sip_svc_get_c_idx(ctrl, c_token);
	if (c_idx == SIP_SVC_ID_INVALID) {
		k_mutex_unlock(&ctrl->data_mutex);
		return -EINVAL;
	}

	if (ctrl->clients[c_idx].id == SIP_SVC_ID_INVALID) {
		k_mutex_unlock(&ctrl->data_mutex);
		return -ENODATA;
	}

	if (ctrl->clients[c_idx].active_trans_cnt != 0) {
		k_mutex_unlock(&ctrl->data_mutex);
		return -EBUSY;
	}

	if (ctrl->clients[c_idx].state != SIP_SVC_CLIENT_ST_IDLE) {
		k_mutex_unlock(&ctrl->data_mutex);
		return -ECANCELED;
	}

	LOG_INF("Unregister the client channel 0x%x", ctrl->clients[c_idx].token);
	ctrl->clients[c_idx].id = SIP_SVC_ID_INVALID;
	ctrl->clients[c_idx].state = SIP_SVC_CLIENT_ST_INVALID;
	ctrl->clients[c_idx].token = SIP_SVC_ID_INVALID;
	ctrl->clients[c_idx].priv_data = NULL;
	sip_svc_id_mgr_free(ctrl->client_id_pool, c_idx);

	k_mutex_unlock(&ctrl->data_mutex);
	return 0;
}