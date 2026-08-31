void modem_pipe_attach(struct modem_pipe *pipe, modem_pipe_api_callback callback, void *user_data)
{
	pipe_set_callback(pipe, callback, user_data);

	if (pipe_test_events(pipe, PIPE_EVENT_RECEIVE_READY_BIT)) {
		pipe_call_callback(pipe, MODEM_PIPE_EVENT_RECEIVE_READY);
	}

	if (pipe_test_events(pipe, PIPE_EVENT_TRANSMIT_IDLE_BIT)) {
		pipe_call_callback(pipe, MODEM_PIPE_EVENT_TRANSMIT_IDLE);
	}
}