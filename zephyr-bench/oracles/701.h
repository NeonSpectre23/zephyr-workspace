static inline int zbus_async_listener_set_work_queue(const struct zbus_observer *obs,
						     struct k_work_q *queue)
{
	CHECKIF(obs == NULL) {
		return -EINVAL;
	}

	CHECKIF(obs->type != ZBUS_OBSERVER_ASYNC_LISTENER_TYPE) {
		return -EINVAL;
	}

	CHECKIF(queue == NULL) {
		return -EINVAL;
	}

	static struct k_spinlock zbus_async_listener_slock;

	K_SPINLOCK(&zbus_async_listener_slock) {
		struct zbus_async_listener_work *async_listener =
			CONTAINER_OF(obs->work, struct zbus_async_listener_work, work);

		async_listener->queue = queue;
	}
	return 0;
}