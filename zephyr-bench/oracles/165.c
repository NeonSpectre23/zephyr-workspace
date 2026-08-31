int modem_pipe_close_async(struct modem_pipe *pipe)
{
	if (pipe_test_events(pipe, PIPE_EVENT_CLOSED_BIT)) {
		pipe_call_callback(pipe, MODEM_PIPE_EVENT_CLOSED);
		return 0;
	}

	return pipe_call_close(pipe);
}