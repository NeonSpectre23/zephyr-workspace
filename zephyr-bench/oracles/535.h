static inline int modem_chat_script_run(struct modem_chat *chat,
					const struct modem_chat_script *script)
{
	return modem_chat_run_script_async(chat, script);
}