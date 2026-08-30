int settings_runtime_set(const char *name, const void *data, size_t len)
{
	struct settings_handler_static *ch;
	const char *name_key;
	struct read_cb_arg arg;

	ch = settings_parse_and_lookup(name, &name_key);
	if (!ch) {
		return -EINVAL;
	}

	arg.data = data;
	arg.len = len;
	return ch->h_set(name_key, len, settings_runtime_read_cb, (void *)&arg);
}