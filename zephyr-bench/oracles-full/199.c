void modem_chat_script_init(struct modem_chat_script *script)
{
	memset(script, 0, sizeof(struct modem_chat_script));
	script->name = "";
}