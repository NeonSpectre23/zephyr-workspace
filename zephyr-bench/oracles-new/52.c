static int cleanup(void)
{
	int err;

	for (size_t i = 0; i < cig_create_param.num_cis; i++) {
		(void)k_work_cancel_delayable(&iso_chans[i].send_work);
	}

	err = k_sem_take(&sem_disconnected, K_NO_WAIT);
	if (err != 0) {
		for (int i = 0; i < cig_create_param.num_cis; i++) {
			err = k_sem_take(&sem_iso_disconnected, K_NO_WAIT);
			if (err == 0) {
				err = bt_iso_chan_disconnect(&iso_chans[i].chan);
				if (err != 0) {
					LOG_ERR("Could not disconnect ISO[%d]: %d",
						i, err);
					break;
				}
			} /* else ISO already disconnected */
		}

		err = bt_conn_disconnect(default_conn,
					 BT_HCI_ERR_REMOTE_USER_TERM_CONN);
		if (err != 0) {
			LOG_ERR("Could not disconnect ACL: %d", err);
			return err;
		}

		err = k_sem_take(&sem_disconnected, K_FOREVER);
		if (err != 0) {
			LOG_ERR("failed to take sem_disconnected: %d", err);
			return err;
		}
	} /* else ACL already disconnected */

	if (cig) {
		err = bt_iso_cig_terminate(cig);
		if (err != 0) {
			LOG_ERR("Could not terminate CIG: %d", err);
			return err;
		}
		cig = NULL;
	}

	return err;
}