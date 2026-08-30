int modem_pipe_receive(struct modem_pipe *pipe, uint8_t *buf, size_t size)
{
	if (!pipe_test_events(pipe, PIPE_EVENT_OPENED_BIT)) {
		return 0;
	}

	pipe_clear_events(pipe, PIPE_EVENT_RECEIVE_READY_BIT);
	return pipe_call_receive(pipe, buf, size);
}