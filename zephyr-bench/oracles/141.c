int modem_chat_init(struct modem_chat *chat, const struct modem_chat_config *config)
{
	__ASSERT_NO_MSG(chat != NULL);
	__ASSERT_NO_MSG(config != NULL);
	__ASSERT_NO_MSG(config->receive_buf != NULL);
	__ASSERT_NO_MSG(config->receive_buf_size > 0);
	__ASSERT_NO_MSG(config->argv != NULL);
	__ASSERT_NO_MSG(config->argv_size > 0);
	__ASSERT_NO_MSG(config->delimiter != NULL);
	__ASSERT_NO_MSG(config->delimiter_size > 0);
	__ASSERT_NO_MSG(!((config->filter == NULL) && (config->filter_size > 0)));
	__ASSERT_NO_MSG(!((config->unsol_matches == NULL) && (config->unsol_matches_size > 0)));

	memset(chat, 0x00, sizeof(*chat));
	chat->pipe = NULL;
	chat->user_data = config->user_data;
	chat->receive_buf = config->receive_buf;
	chat->receive_buf_size = config->receive_buf_size;
	chat->argv = config->argv;
	chat->argv_size = config->argv_size;
	chat->delimiter = config->delimiter;
	chat->delimiter_size = config->delimiter_size;
	chat->filter = config->filter;
	chat->filter_size = config->filter_size;
	chat->matches[MODEM_CHAT_MATCHES_INDEX_UNSOL] = config->unsol_matches;
	chat->matches_size[MODEM_CHAT_MATCHES_INDEX_UNSOL] = config->unsol_matches_size;
	atomic_set(&chat->script_state, 0);
	k_sem_init(&chat->script_stopped_sem, 0, 1);
	k_work_init(&chat->receive_work, modem_chat_process_handler);
	k_work_init(&chat->script_run_work, modem_chat_script_run_handler);
	k_work_init_delayable(&chat->script_timeout_work, modem_chat_script_timeout_handler);
	k_work_init(&chat->script_abort_work, modem_chat_script_abort_handler);
	k_work_init(&chat->script_send_work, modem_chat_script_send_handler);
	k_work_init_delayable(&chat->script_send_timeout_work,
			      modem_chat_script_send_timeout_handler);

#if CONFIG_MODEM_STATS
	init_buf_stats(chat);
#endif

	return 0;
}