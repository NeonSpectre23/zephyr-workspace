bool zbus_iterate_over_channels_with_user_data(
	bool (*iterator_func)(const struct zbus_channel *chan, void *user_data), void *user_data)
{
	STRUCT_SECTION_FOREACH(zbus_channel, chan) {
		if (!(*iterator_func)(chan, user_data)) {
			return false;
		}
	}
	return true;
}