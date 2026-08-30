int modem_pipe_transmit(struct modem_pipe *pipe, const uint8_t *buf, size_t size)
{
	if (!pipe_test_events(pipe, PIPE_EVENT_OPENED_BIT)) {
		return 0;
	}

	pipe_clear_events(pipe, PIPE_EVENT_TRANSMIT_IDLE_BIT);
	return pipe_call_transmit(pipe, buf, size);
}