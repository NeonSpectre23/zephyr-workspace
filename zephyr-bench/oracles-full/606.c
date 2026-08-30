bool zbus_iterate_over_observers_with_user_data(
	bool (*iterator_func)(const struct zbus_observer *obs, void *user_data), void *user_data)
{
	STRUCT_SECTION_FOREACH(zbus_observer, obs) {
		if (!(*iterator_func)(obs, user_data)) {
			return false;
		}
	}
	return true;
}