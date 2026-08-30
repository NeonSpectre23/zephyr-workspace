int zbus_obs_set_enable(const struct zbus_observer *obs, bool enabled)
{
	_ZBUS_ASSERT(obs != NULL, "obs is required");

	K_SPINLOCK(&obs_slock) {
		if (obs->data->enabled != enabled) {
			obs->data->enabled = enabled;

			update_all_channels_hop(obs);
		}
	}

	return 0;
}