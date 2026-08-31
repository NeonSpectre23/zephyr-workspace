int modem_chat_match_set_match(struct modem_chat_match *chat_match, const char *match)
{
	size_t size;

	size = strnlen(match, UINT8_MAX + 1);

	if (size == (UINT8_MAX + 1)) {
		return -ENOMEM;
	}

	chat_match->match = match;
	chat_match->match_size = (uint8_t)size;
	return 0;
}