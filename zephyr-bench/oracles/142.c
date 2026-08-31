bool modem_chat_is_running(struct modem_chat *chat)
{
	return atomic_test_bit(&chat->script_state, MODEM_CHAT_SCRIPT_STATE_RUNNING_BIT);
}