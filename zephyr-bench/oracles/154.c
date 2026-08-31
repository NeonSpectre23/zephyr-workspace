int modem_chat_script_set_script_chats(struct modem_chat_script *script,
				       const struct modem_chat_script_chat *script_chats,
				       uint16_t script_chats_size)
{
	if (!modem_chat_validate_array(script_chats, script_chats_size)) {
		return -EINVAL;
	}

	script->script_chats = script_chats;
	script->script_chats_size = script_chats_size;
	return 0;
}