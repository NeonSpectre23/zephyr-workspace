int sip_svc_close(void *ct, uint32_t c_token, struct sip_svc_request *pre_close_req)
{
	uint32_t c_idx;
	int err;

	if (ct == NULL || !is_sip_svc_controller(ct)) {
		return -EINVAL;
	}

	struct sip_svc_controller *ctrl = (struct sip_svc_controller *)ct;

	/*If pre-close request is provided, send it to lower layers*/
	if (pre_close_req != NULL) {
		err = sip_svc_send(ct, c_token, pre_close_req, NULL);
		if (err < 0) {
			LOG_ERR("Error sending pre_close_req : %d", err);
			return -ENOTSUP;
		}
	}

	err = k_mutex_lock(&ctrl->data_mutex, K_FOREVER);
	if (err != 0) {
		LOG_ERR("Error in acquiring lock %d", err);
		return err;
	}

	c_idx = sip_svc_get_c_idx(ctrl, c_token);
	if (c_idx == SIP_SVC_ID_INVALID) {
		k_mutex_unlock(&ctrl->data_mutex);
		return -EINVAL;
	}

	if (ctrl->clients[c_idx].state != SIP_SVC_CLIENT_ST_OPEN) {
		LOG_ERR("Client is in wrong state  %d", ctrl->clients[c_idx].state);
		k_mutex_unlock(&ctrl->data_mutex);
		return -EPROTO;
	}

	if (ctrl->clients[c_idx].active_trans_cnt != 0) {
		ctrl->clients[c_idx].state = SIP_SVC_CLIENT_ST_ABORT;
	} else {
		ctrl->clients[c_idx].state = SIP_SVC_CLIENT_ST_IDLE;
	}

#if CONFIG_ARM_SIP_SVC_SUBSYS_SINGLY_OPEN
	(void)atomic_set(&ctrl->open_lock, SIP_SVC_OPEN_UNLOCKED);
#endif
	k_mutex_unlock(&ctrl->data_mutex);

	LOG_INF("Close the client channel 0x%x", ctrl->clients[c_idx].token);
	return 0;
}