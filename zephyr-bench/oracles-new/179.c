int input_report(const struct device *dev,
		 uint8_t type, uint16_t code, int32_t value, bool sync,
		 k_timeout_t timeout)
{
	struct input_event evt = {
		.dev = dev,
		.sync = sync,
		.type = type,
		.code = code,
		.value = value,
	};

#ifdef CONFIG_INPUT_MODE_THREAD
	int ret;

	if (!K_TIMEOUT_EQ(timeout, K_NO_WAIT) &&
	    k_current_get() == k_work_queue_thread_get(&k_sys_work_q)) {
		LOG_DBG("Timeout discarded. No blocking in syswq.");
		timeout = K_NO_WAIT;
	}

	ret = k_msgq_put(&input_msgq, &evt, timeout);
	if (ret < 0) {
		LOG_WRN("Event dropped, queue full, not blocking in syswq.");
		return ret;
	}

	return 0;
#else
	input_process(&evt);
	return 0;
#endif
}