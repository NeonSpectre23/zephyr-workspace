static inline const char *zbus_obs_name(const struct zbus_observer *obs)
{
	__ASSERT(obs != NULL, "obs is required");

	return obs->name;
}