int zbus_chan_add_obs_with_node(const struct zbus_channel *chan, const struct zbus_observer *obs,
				struct zbus_observer_node *node, k_timeout_t timeout)
{
	int err;

	/* On success the channel semaphore has been taken */
	err = _zbus_runtime_take_chan_sem_and_obs_check(chan, obs, timeout);
	if (err) {
		return err;
	}

	if (node->chan != NULL) {
		k_sem_give(&chan->data->sem);

		return -EBUSY;
	}

	node->obs = obs;
	node->chan = chan;

	sys_slist_append(&chan->data->observers, &node->node);

	k_sem_give(&chan->data->sem);

	return 0;
}