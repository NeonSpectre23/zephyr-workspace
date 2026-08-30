void modem_pipe_notify_closed(struct modem_pipe *pipe)
{
	pipe_set_events(pipe, PIPE_EVENT_TRANSMIT_IDLE_BIT | PIPE_EVENT_CLOSED_BIT);
	pipe_call_callback(pipe, MODEM_PIPE_EVENT_CLOSED);
}