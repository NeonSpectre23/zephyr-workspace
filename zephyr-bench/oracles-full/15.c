static int cleanup(struct bt_le_per_adv_sync *sync, struct bt_iso_big *big)
{
	int pa_err = 0;
	int big_err = 0;

	if (!per_adv_lost && sync) {
		LOG_INF("Deleting Periodic advertisement Sync");
		pa_err = bt_le_per_adv_sync_delete(sync);
		if (pa_err != 0) {
			LOG_ERR("Failed to delete Periodic advertisement sync (err %d)",
				pa_err);
		}
	}

	if (!big_sync_lost && big) {
		LOG_INF("Terminating BIG Sync");
		big_err = bt_iso_big_terminate(big);
		if (big_err != 0) {
			LOG_ERR("BIG terminate failed (err %d)", big_err);
		}
	}

	if (pa_err != 0 || big_err != 0) {
		LOG_ERR("Cleanup failed (%d), recommend restart application to "
		       "avoid any potential leftovers",
		       pa_err != 0 ? pa_err : big_err);
	}

	return pa_err != 0 ? pa_err : big_err;
}