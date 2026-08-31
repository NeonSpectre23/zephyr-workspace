int zbus_chan_rm_obs(const struct zbus_channel *chan, const struct zbus_observer *obs,
		     k_timeout_t timeout)
{
	int err;
	struct zbus_observer_node *obs_nd, *tmp;
	struct zbus_observer_node *prev_obs_nd = NULL;

	_ZBUS_ASSERT(!k_is_in_isr(), "ISR blocked");
	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(obs != NULL, "obs is required");

	err = k_sem_take(&chan->data->sem, timeout);
	if (err) {
		return err;
	}

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&chan->data->observers, obs_nd, tmp, node) {
		if (obs_nd->obs == obs) {
			sys_slist_remove(&chan->data->observers,
					 prev_obs_nd ? &prev_obs_nd->node : NULL,
					 &obs_nd->node);
#if defined(CONFIG_ZBUS_RUNTIME_OBSERVERS_NODE_ALLOC_NONE)
			obs_nd->chan = NULL;
#else
			_zbus_runtime_observer_node_free(obs_nd);
#endif

			k_sem_give(&chan->data->sem);

			return 0;
		}

		prev_obs_nd = obs_nd;
	}

	k_sem_give(&chan->data->sem);

	return -ENODATA;
}