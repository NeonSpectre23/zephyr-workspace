int modem_cmux_disconnect(struct modem_cmux *cmux)
{
	int ret;

	ret = modem_cmux_disconnect_async(cmux);
	if (ret < 0) {
		return ret;
	}

	if (!wait_state(cmux, MODEM_CMUX_STATE_DISCONNECTED, MODEM_CMUX_T2_TIMEOUT)) {
		return -EAGAIN;
	}

	return 0;
}