void modem_pipe_notify_transmit_idle(struct modem_pipe *pipe)
{
	pipe_post_events(pipe, PIPE_EVENT_TRANSMIT_IDLE_BIT);
	pipe_call_callback(pipe, MODEM_PIPE_EVENT_TRANSMIT_IDLE);
}