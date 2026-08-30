int bt_le_scan_stop(void)
{
	bt_scan_softreset();
	scan_dev_found_cb = NULL;

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    atomic_test_and_clear_bit(bt_dev.flags, BT_DEV_SCAN_LIMITED)) {
		atomic_clear_bit(bt_dev.flags, BT_DEV_RPA_VALID);

#if defined(CONFIG_BT_SMP)
		bt_id_pending_keys_update();
#endif
	}

	return bt_le_scan_user_remove(BT_LE_SCAN_USER_EXPLICIT_SCAN);
}