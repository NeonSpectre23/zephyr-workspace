void *sip_svc_get_priv_data(void *ct, uint32_t c_token)
{
	uint32_t c_idx;
	int err;

	if (ct == NULL || !is_sip_svc_controller(ct)) {
		return NULL;
	}

	struct sip_svc_controller *ctrl = (struct sip_svc_controller *)ct;

	err = k_mutex_lock(&ctrl->data_mutex, K_FOREVER);
	if (err != 0) {
		LOG_ERR("Failed to get lock %d", err);
		return NULL;
	}

	c_idx = sip_svc_get_c_idx(ctrl, c_token);
	if (c_idx == SIP_SVC_ID_INVALID) {
		LOG_ERR("Client id is invalid");
		k_mutex_unlock(&ctrl->data_mutex);
		return NULL;
	}

	k_mutex_unlock(&ctrl->data_mutex);
	return ctrl->clients[c_idx].priv_data;
}