int modem_chat_run_script(struct modem_chat *chat, const struct modem_chat_script *script)
{
	int ret;

	ret = modem_chat_run_script_async(chat, script);
	if (ret < 0) {
		return ret;
	}

	ret = k_sem_take(&chat->script_stopped_sem, K_FOREVER);
	if (ret < 0) {
		return ret;
	}

	return (chat->script_result == MODEM_CHAT_SCRIPT_RESULT_SUCCESS) ? 0 : -EAGAIN;
}