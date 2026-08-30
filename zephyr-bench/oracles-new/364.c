void modem_pipelink_init(struct modem_pipelink *link, struct modem_pipe *pipe)
{
	link->pipe = pipe;
	link->callback = NULL;
	link->user_data = NULL;
	link->connected = false;
}