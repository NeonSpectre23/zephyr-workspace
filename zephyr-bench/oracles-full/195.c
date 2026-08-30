void modem_chat_script_abort(struct modem_chat *chat)
{
	modem_work_submit(&chat->script_abort_work);
}