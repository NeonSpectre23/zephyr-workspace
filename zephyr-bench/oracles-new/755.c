int sip_svc_send(void *ct, uint32_t c_token, struct sip_svc_request *request, sip_svc_cb_fn cb)
{
	uint32_t trans_id = SIP_SVC_ID_INVALID;
	uint32_t trans_idx = SIP_SVC_ID_INVALID;
	uint32_t c_idx;
	int ret;

	if (ct == NULL || !is_sip_svc_controller(ct) || request == NULL) {
		return -EINVAL;
	}

	struct sip_svc_controller *ctrl = (struct sip_svc_controller *)ct;

	if (!sip_svc_plat_func_id_valid(ctrl->dev,
					(uint32_t)SIP_SVC_PROTO_HEADER_GET_CODE(request->header),
					(uint32_t)request->a0)) {
		return -EOPNOTSUPP;
	}

	ret = k_mutex_lock(&ctrl->data_mutex, K_FOREVER);
	if (ret != 0) {
		LOG_ERR("Failed to get lock %d", ret);
		return -ENOLCK;
	}

	c_idx = sip_svc_get_c_idx(ctrl, c_token);
	if (c_idx == SIP_SVC_ID_INVALID) {
		k_mutex_unlock(&ctrl->data_mutex);
		return -EINVAL;
	}

	if (ctrl->clients[c_idx].state != SIP_SVC_CLIENT_ST_OPEN) {
		k_mutex_unlock(&ctrl->data_mutex);
		return -ESRCH;
	}

	/* Allocate a trans id for the request */
	trans_idx = sip_svc_id_mgr_alloc(ctrl->clients[c_idx].trans_idx_pool);
	if (trans_idx == SIP_SVC_ID_INVALID) {
		LOG_ERR("Fail to allocate transaction id");
		k_mutex_unlock(&ctrl->data_mutex);
		return -ENOMEM;
	}

	trans_id = sip_svc_plat_format_trans_id(ctrl->dev, c_idx, trans_idx);
	/* Additional check for an unsupported condition*/
	if (((int)trans_id) < 0) {
		LOG_ERR("Unsupported condition, trans_id < 0");
		sip_svc_id_mgr_free(ctrl->clients[c_idx].trans_idx_pool, trans_idx);
		k_mutex_unlock(&ctrl->data_mutex);
		return -ENOTSUP;
	}

	/* Assign the trans id of this request */
	SIP_SVC_PROTO_HEADER_SET_TRANS_ID(request->header, trans_id);

	/* Map trans id to client, callback, response data addr */
	if (sip_svc_id_map_insert_item(ctrl->trans_id_map, trans_id, (void *)cb,
				       (void *)((request->resp_data_addr >> 32) & 0xFFFFFFFF),
				       (void *)(request->resp_data_addr & 0xFFFFFFFF),
				       (void *)(uint64_t)request->resp_data_size,
				       request->priv_data, (void *)(uint64_t)c_idx) != 0) {

		LOG_ERR("Fail to insert transaction id to map");
		sip_svc_id_mgr_free(ctrl->clients[c_idx].trans_idx_pool, trans_idx);
		k_mutex_unlock(&ctrl->data_mutex);
		return -ENOMSG;
	}

	/* Insert request to MSGQ */
	LOG_INF("send command to msgq");
	if (k_msgq_put(&ctrl->req_msgq, (void *)request, K_NO_WAIT) != 0) {
		LOG_ERR("Request msgq full");
		sip_svc_id_map_remove_item(ctrl->trans_id_map, trans_id);
		sip_svc_id_mgr_free(ctrl->clients[c_idx].trans_idx_pool, trans_idx);
		k_mutex_unlock(&ctrl->data_mutex);
		return -ENOBUFS;
	}
	++ctrl->clients[c_idx].active_trans_cnt;

	if (!ctrl->tid) {
		LOG_ERR("Thread not spawned during init");
		sip_svc_id_map_remove_item(ctrl->trans_id_map, trans_id);
		sip_svc_id_mgr_free(ctrl->clients[c_idx].trans_idx_pool, trans_idx);
		k_mutex_unlock(&ctrl->data_mutex);
		return -EHOSTDOWN;
	}

	LOG_INF("Wakeup sip_svc thread");
	k_thread_resume(ctrl->tid);
	k_mutex_unlock(&ctrl->data_mutex);

	return (int)trans_id;
}