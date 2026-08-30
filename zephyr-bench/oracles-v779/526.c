int zbus_obs_attach_to_thread(const struct zbus_observer *obs)
{
	_ZBUS_ASSERT(!k_is_in_isr(), "cannot attach to an ISR");
	_ZBUS_ASSERT(obs != NULL, "obs is required");

	int current_thread_priority = k_thread_priority_get(k_current_get());

	K_SPINLOCK(&obs_slock) {
		if (obs->data->priority != current_thread_priority) {
			obs->data->priority = current_thread_priority;

			update_all_channels_hop(obs);
		}
	}

	return 0;
}