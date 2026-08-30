void modem_pipe_notify_opened(struct modem_pipe *pipe)
{
	pipe_set_events(pipe, PIPE_EVENT_OPENED_BIT | PIPE_EVENT_TRANSMIT_IDLE_BIT);
	pipe_call_callback(pipe, MODEM_PIPE_EVENT_OPENED);
	pipe_call_callback(pipe, MODEM_PIPE_EVENT_TRANSMIT_IDLE);
}