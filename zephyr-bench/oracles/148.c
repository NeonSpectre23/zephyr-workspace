int modem_chat_run_script_async(struct modem_chat *chat, const struct modem_chat_script *script)
{
	bool script_is_running;

	if (chat->pipe == NULL) {
		return -EPERM;
	}

	/* Validate script */
	if (script->script_chats == NULL ||
	   (script->script_chats_size == 0
	    && script->script_chats != modem_chat_empty_script_chats) ||
	   (script->abort_matches_size == 0
	    && script->abort_matches != NULL
	    && script->abort_matches != modem_chat_empty_matches)) {
		return -EINVAL;
	}

	/* Validate script commands */
	for (uint16_t i = 0; i < script->script_chats_size; i++) {
		if ((script->script_chats[i].request_size == 0) &&
		    (script->script_chats[i].response_matches_size == 0) &&
		    (script->script_chats[i].timeout == 0)) {
			return -EINVAL;
		}
	}

	script_is_running =
		atomic_test_and_set_bit(&chat->script_state, MODEM_CHAT_SCRIPT_STATE_RUNNING_BIT);

	if (script_is_running == true) {
		return -EBUSY;
	}

	k_sem_reset(&chat->script_stopped_sem);

	chat->pending_script = script;
	modem_work_submit(&chat->script_run_work);
	return 0;
}