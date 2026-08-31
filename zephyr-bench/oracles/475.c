bool zbus_iterate_over_channels(bool (*iterator_func)(const struct zbus_channel *chan))
{
	STRUCT_SECTION_FOREACH(zbus_channel, chan) {
		if (!(*iterator_func)(chan)) {
			return false;
		}
	}
	return true;
}