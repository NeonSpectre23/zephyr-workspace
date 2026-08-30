void modem_pipe_release(struct modem_pipe *pipe)
{
	pipe_set_callback(pipe, NULL, NULL);
}