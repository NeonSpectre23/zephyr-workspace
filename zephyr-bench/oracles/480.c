int zbus_obs_detach_from_thread(const struct zbus_observer *obs)
{
	_ZBUS_ASSERT(!k_is_in_isr(), "cannot detach from an ISR");
	_ZBUS_ASSERT(obs != NULL, "obs is required");

	K_SPINLOCK(&obs_slock) {
		obs->data->priority = ZBUS_MIN_THREAD_PRIORITY;

		update_all_channels_hop(obs);
	}

	return 0;
}