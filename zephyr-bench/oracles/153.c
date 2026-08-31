int modem_chat_script_set_abort_matches(struct modem_chat_script *script,
					const struct modem_chat_match *abort_matches,
					uint16_t abort_matches_size)
{
	if (!modem_chat_validate_array(abort_matches, abort_matches_size)) {
		return -EINVAL;
	}

	script->abort_matches = abort_matches;
	script->abort_matches_size = abort_matches_size;
	return 0;
}