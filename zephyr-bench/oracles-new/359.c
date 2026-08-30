int modem_pipe_open_async(struct modem_pipe *pipe)
{
	if (pipe_test_events(pipe, PIPE_EVENT_OPENED_BIT)) {
		pipe_call_callback(pipe, MODEM_PIPE_EVENT_OPENED);
		return 0;
	}

	return pipe_call_open(pipe);
}