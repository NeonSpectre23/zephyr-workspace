int modem_chat_attach(struct modem_chat *chat, struct modem_pipe *pipe)
{
	chat->pipe = pipe;
	modem_chat_parse_reset(chat);
	modem_pipe_attach(chat->pipe, modem_chat_pipe_callback, chat);
	return 0;
}