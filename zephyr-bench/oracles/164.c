int modem_pipe_close(struct modem_pipe *pipe, k_timeout_t timeout)
{
	int ret;

	if (pipe_test_events(pipe, PIPE_EVENT_CLOSED_BIT)) {
		return 0;
	}

	ret = pipe_call_close(pipe);
	if (ret < 0) {
		return ret;
	}

	if (!pipe_await_events(pipe, PIPE_EVENT_CLOSED_BIT, timeout)) {
		return -EAGAIN;
	}

	return 0;
}