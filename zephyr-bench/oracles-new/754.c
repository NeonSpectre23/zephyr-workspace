uint32_t sip_svc_register(void *ct, void *priv_data)
{
	int err;
	uint32_t c_idx = SIP_SVC_ID_INVALID;

	if (ct == NULL || !is_sip_svc_controller(ct)) {
		return SIP_SVC_ID_INVALID;
	}

	struct sip_svc_controller *ctrl = (struct sip_svc_controller *)ct;

	err = k_mutex_lock(&ctrl->data_mutex, K_FOREVER);
	if (err != 0) {
		LOG_ERR("Error in acquiring mutex %d", err);
		return SIP_SVC_ID_INVALID;
	}

	c_idx = sip_svc_id_mgr_alloc(ctrl->client_id_pool);
	if (c_idx != SIP_SVC_ID_INVALID) {
		ctrl->clients[c_idx].id = c_idx;
		ctrl->clients[c_idx].token = sip_svc_generate_c_token();
		ctrl->clients[c_idx].state = SIP_SVC_CLIENT_ST_IDLE;
		ctrl->clients[c_idx].priv_data = priv_data;
		k_mutex_unlock(&ctrl->data_mutex);
		LOG_INF("Register the client channel 0x%x", ctrl->clients[c_idx].token);
		return ctrl->clients[c_idx].token;
	}

	k_mutex_unlock(&ctrl->data_mutex);
	return SIP_SVC_ID_INVALID;
}