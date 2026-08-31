int zbus_chan_add_obs(const struct zbus_channel *chan, const struct zbus_observer *obs,
		      k_timeout_t timeout)
{
	int err;
	k_timepoint_t end_time = sys_timepoint_calc(timeout);

	/* On success the channel semaphore has been taken */
	err = _zbus_runtime_take_chan_sem_and_obs_check(chan, obs, timeout);
	if (err) {
		return err;
	}

	struct zbus_observer_node *new_obs_nd = NULL;

	err = _zbus_runtime_observer_node_alloc(&new_obs_nd, sys_timepoint_timeout(end_time));
	if (err) {
		k_sem_give(&chan->data->sem);

		return err;
	}

	new_obs_nd->obs = obs;

	sys_slist_append(&chan->data->observers, &new_obs_nd->node);

	k_sem_give(&chan->data->sem);

	return 0;
}