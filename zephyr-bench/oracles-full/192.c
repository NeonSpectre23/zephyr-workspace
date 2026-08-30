int modem_chat_match_set_separators(struct modem_chat_match *chat_match, const char *separators)
{
	size_t size;

	size = strnlen(separators, UINT8_MAX + 1);

	if (size == (UINT8_MAX + 1)) {
		return -ENOMEM;
	}

	chat_match->separators = separators;
	chat_match->separators_size = (uint8_t)size;
	return 0;
}