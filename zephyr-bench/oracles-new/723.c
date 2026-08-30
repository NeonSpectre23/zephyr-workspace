int settings_runtime_commit(const char *name)
{
	struct settings_handler_static *ch;
	const char *name_key;

	ch = settings_parse_and_lookup(name, &name_key);
	if (!ch) {
		return -EINVAL;
	}

	if (ch->h_commit) {
		return ch->h_commit();
	} else {
		return 0;
	}
}