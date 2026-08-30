void platformRadioInit(void)
{
	struct ieee802154_config cfg;

	dataInit();

	__ASSERT_NO_MSG(device_is_ready(radio_dev));

	radio_api = (struct ieee802154_radio_api *)radio_dev->api;
	if (!radio_api) {
		return;
	}

	k_work_queue_start(&ot_work_q, ot_task_stack,
			   K_KERNEL_STACK_SIZEOF(ot_task_stack),
			   OT_WORKER_PRIORITY, NULL);
	k_thread_name_set(&ot_work_q.thread, "ot_radio_workq");

	if ((radio_api->get_capabilities(radio_dev) &
	     IEEE802154_HW_TX_RX_ACK) != IEEE802154_HW_TX_RX_ACK) {
		LOG_ERR("Only radios with automatic ack handling "
			"are currently supported");
		k_panic();
	}

	cfg.event_handler = handle_radio_event;
	radio_api->configure(radio_dev, IEEE802154_CONFIG_EVENT_HANDLER, &cfg);
}