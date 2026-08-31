int modem_cmux_disconnect_async(struct modem_cmux *cmux)
{
	int ret = 0;

	if (cmux->state == MODEM_CMUX_STATE_DISCONNECTED) {
		return -EALREADY;
	}

	K_SPINLOCK(&cmux->work_lock) {
		if (!cmux->attached) {
			ret = -EPERM;
			K_SPINLOCK_BREAK;
		}

		if (k_work_delayable_is_pending(&cmux->disconnect_work) == false) {
			modem_work_schedule(&cmux->disconnect_work, K_NO_WAIT);
		}
	}

	return ret;
}