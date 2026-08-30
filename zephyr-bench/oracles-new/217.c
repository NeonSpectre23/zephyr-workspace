bool k_p4wq_cancel(struct k_p4wq *queue, struct k_p4wq_work *item)
{
	k_spinlock_key_t k = k_spin_lock(&queue->lock);
	bool ret = rb_contains(&queue->queue, &item->rbnode);

	if (ret) {
		rb_remove(&queue->queue, &item->rbnode);

		if (queue->done_handler) {
			k_spin_unlock(&queue->lock, k);
			queue->done_handler(item);
			k = k_spin_lock(&queue->lock);
		} else {
			k_sem_give(&item->done_sem);
		}
	}

	k_spin_unlock(&queue->lock, k);
	return ret;
}