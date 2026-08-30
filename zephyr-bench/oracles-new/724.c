int settings_runtime_get(const char *name, void *data, size_t len)
{
	struct settings_handler_static *ch;
	const char *name_key;

	ch = settings_parse_and_lookup(name, &name_key);
	if (!ch) {
		return -EINVAL;
	}

	if (!ch->h_get) {
		return -ENOTSUP;
	}

	return ch->h_get(name_key, data, len);
}