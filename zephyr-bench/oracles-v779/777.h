static inline int zbus_obs_is_enabled(const struct zbus_observer *obs, bool *enable)
{
	_ZBUS_ASSERT(obs != NULL, "obs is required");
	_ZBUS_ASSERT(enable != NULL, "enable is required");

	*enable = obs->data->enabled;

	return 0;
}