int sip_svc_open(void *ct, uint32_t c_token, k_timeout_t k_timeout)
{

	uint32_t c_idx;
	int ret;
	struct k_timer timer;

	if (ct == NULL || !is_sip_svc_controller(ct)) {
		return -EINVAL;
	}

	struct sip_svc_controller *ctrl = (struct sip_svc_controller *)ct;

	/* Initialize the timer */
	k_timer_init(&timer, NULL, NULL);

	/**
	 * Run through the loop until the client is in IDLE state.
	 * Then move the client state to open. If the client has any pending transactions,
	 * the client state will be ABORT state.This will only change when the pending
	 * transactions are complete.
	 */
	for (bool first_iteration = false; get_timer_status(&first_iteration, &timer, k_timeout);
	     k_usleep(CONFIG_ARM_SIP_SVC_SUBSYS_ASYNC_POLLING_DELAY)) {

		ret = k_mutex_lock(&ctrl->data_mutex, K_NO_WAIT);
		if (ret != 0) {
			LOG_WRN("0x%x didn't get data lock", c_token);
			continue;
		}

		c_idx = sip_svc_get_c_idx(ctrl, c_token);
		if (c_idx == SIP_SVC_ID_INVALID) {
			LOG_ERR("Invalid client token");
			k_mutex_unlock(&ctrl->data_mutex);
			k_timer_stop(&timer);
			return -EINVAL;
		}

		/* Check if the state of client is already open state*/
		if (ctrl->clients[c_idx].state == SIP_SVC_CLIENT_ST_OPEN) {
			LOG_DBG("client with token 0x%x is already open", c_token);
			k_mutex_unlock(&ctrl->data_mutex);
			k_timer_stop(&timer);
			return -EALREADY;
		}

		/* Check if the state of client is in idle state*/
		if (ctrl->clients[c_idx].state != SIP_SVC_CLIENT_ST_IDLE) {
			LOG_DBG("client with token 0x%x is not idle", c_token);
			k_mutex_unlock(&ctrl->data_mutex);
			continue;
		}

#if CONFIG_ARM_SIP_SVC_SUBSYS_SINGLY_OPEN
		/**
		 * Acquire open lock, when only one client can transact at
		 * a time.
		 */
		if (!atomic_cas(&ctrl->open_lock, SIP_SVC_OPEN_UNLOCKED, SIP_SVC_OPEN_LOCKED)) {
			LOG_DBG("0x%x didn't get open lock, wait for it to be released", c_token);
			k_mutex_unlock(&ctrl->data_mutex);
			continue;
		}
#endif

		/* Make the client state to be open and stop timer*/
		ctrl->clients[c_idx].state = SIP_SVC_CLIENT_ST_OPEN;
		LOG_INF("0x%x successfully opened a connection with sip_svc", c_token);
		k_mutex_unlock(&ctrl->data_mutex);
		k_timer_stop(&timer);
		return 0;
	}

	k_timer_stop(&timer);
	LOG_ERR("Timedout at %s for 0x%x", __func__, c_token);
	return -ETIMEDOUT;
}