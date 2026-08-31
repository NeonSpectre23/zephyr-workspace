int modem_chat_script_chat_set_response_matches(struct modem_chat_script_chat *script_chat,
						const struct modem_chat_match *response_matches,
						uint16_t response_matches_size)
{
	if (!modem_chat_validate_array(response_matches, response_matches_size)) {
		return -EINVAL;
	}

	script_chat->response_matches = response_matches;
	script_chat->response_matches_size = response_matches_size;
	return 0;
}