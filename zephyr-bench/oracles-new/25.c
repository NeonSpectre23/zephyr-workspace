int bt_le_scan_start(const struct bt_le_scan_param *param, bt_le_scan_cb_t cb)
{
	int err;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	/* Check that the parameters have valid values */
	if (!valid_le_scan_param(param)) {
		return -EINVAL;
	}

	if (param->type && !bt_id_scan_random_addr_check()) {
		return -EINVAL;
	}

	/* Prevent multiple threads to try to enable explicit scanning at the same time.
	 * That could lead to unwanted overwriting of scan_state.explicit_scan_param.
	 */
	err = k_mutex_lock(&scan_state.scan_explicit_params_mutex, K_NO_WAIT);

	if (err) {
		return err;
	}

	err = scan_check_if_state_allowed(BT_LE_SCAN_USER_EXPLICIT_SCAN);

	if (err) {
		k_mutex_unlock(&scan_state.scan_explicit_params_mutex);
		return err;
	}

	/* store the parameters that were used to start the scanner */
	memcpy(&scan_state.explicit_scan_param, param, sizeof(scan_state.explicit_scan_param));

	scan_dev_found_cb = cb;
	err = bt_le_scan_user_add(BT_LE_SCAN_USER_EXPLICIT_SCAN);
	k_mutex_unlock(&scan_state.scan_explicit_params_mutex);

	return err;
}