void modem_pipe_init(struct modem_pipe *pipe, void *data, const struct modem_pipe_api *api)
{
	__ASSERT_NO_MSG(pipe != NULL);
	__ASSERT_NO_MSG(data != NULL);
	__ASSERT_NO_MSG(api != NULL);

	pipe->data = data;
	pipe->api = api;
	pipe->callback = NULL;
	pipe->user_data = NULL;
	k_event_init(&pipe->event);
}