int modem_chat_script_chat_set_request(struct modem_chat_script_chat *script_chat,
				       const char *request)
{
	size_t size;

	size = strnlen(request, UINT16_MAX + 1);

	if (size == (UINT16_MAX + 1)) {
		return -ENOMEM;
	}

	script_chat->request = request;
	script_chat->request_size = (uint16_t)size;
	return 0;
}