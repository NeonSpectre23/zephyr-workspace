int modem_cmux_connect(struct modem_cmux *cmux)
{
	int ret;

	ret = modem_cmux_connect_async(cmux);
	if (ret < 0) {
		return ret;
	}

	if (!wait_state(cmux, MODEM_CMUX_STATE_CONNECTED, MODEM_CMUX_T2_TIMEOUT)) {
		return -EAGAIN;
	}

	return 0;
}