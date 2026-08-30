int bt_enable(bt_ready_cb_t cb)
{
	int err;

	if (IS_ENABLED(CONFIG_ZTEST) && bt_dev.hci == NULL) {
		LOG_ERR("No DT chosen property for HCI");
		return -ENODEV;
	}

	if (!device_is_ready(bt_dev.hci)) {
		LOG_ERR("HCI driver is not ready");
		return -ENODEV;
	}

	bt_monitor_new_index(BT_MONITOR_TYPE_PRIMARY, BT_HCI_BUS, BT_ADDR_ANY, BT_HCI_NAME);

	atomic_clear_bit(bt_dev.flags, BT_DEV_DISABLE);

	if (atomic_test_and_set_bit(bt_dev.flags, BT_DEV_ENABLE)) {
		return -EALREADY;
	}

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		err = bt_settings_init();
		if (err) {
			return err;
		}
	} else if (IS_ENABLED(CONFIG_BT_DEVICE_NAME_DYNAMIC)) {
		err = bt_set_name(CONFIG_BT_DEVICE_NAME);
		if (err) {
			LOG_WRN("Failed to set device name (%d)", err);
		}
	}

	ready_cb = cb;

	/* Give cmd_sem allowing to send first HCI_Reset cmd, the only
	 * exception is if the controller requests to wait for an
	 * initial Command Complete for NOP.
	 */
	if (!IS_ENABLED(CONFIG_BT_WAIT_NOP)) {
		k_sem_init(&bt_dev.ncmd_sem, 1, 1);
	} else {
		k_sem_init(&bt_dev.ncmd_sem, 0, 1);
	}
	k_fifo_init(&bt_dev.cmd_tx_queue);

#if defined(CONFIG_BT_RECV_WORKQ_BT)
	/* RX thread */
	k_work_queue_init(&bt_workq);
	k_work_queue_start(&bt_workq, rx_thread_stack,
			   CONFIG_BT_RX_STACK_SIZE,
			   K_PRIO_COOP(CONFIG_BT_RX_PRIO), NULL);
	k_thread_name_set(&bt_workq.thread, "BT RX WQ");
#endif

	err = bt_hci_open(bt_dev.hci, bt_hci_recv);
	if (err) {
		LOG_ERR("HCI driver open failed (%d)", err);
		return err;
	}

	bt_monitor_send(BT_MONITOR_OPEN_INDEX, NULL, 0);

	if (!cb) {
		return bt_init();
	}

	k_work_submit(&bt_dev.init);
	return 0;
}