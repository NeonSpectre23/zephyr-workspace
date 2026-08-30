int settings_register(struct settings_handler *handler)
{
	return settings_register_with_cprio(handler, 0);
}