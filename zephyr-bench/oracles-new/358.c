int modem_pipe_open(struct modem_pipe *pipe, k_timeout_t timeout)
{
	int ret;

	if (pipe_test_events(pipe, PIPE_EVENT_OPENED_BIT)) {
		return 0;
	}

	ret = pipe_call_open(pipe);
	if (ret < 0) {
		return ret;
	}

	if (!pipe_await_events(pipe, PIPE_EVENT_OPENED_BIT, timeout)) {
		return -EAGAIN;
	}

	return 0;
}