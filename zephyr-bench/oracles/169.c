void modem_pipe_notify_receive_ready(struct modem_pipe *pipe)
{
	pipe_post_events(pipe, PIPE_EVENT_RECEIVE_READY_BIT);
	pipe_call_callback(pipe, MODEM_PIPE_EVENT_RECEIVE_READY);
}