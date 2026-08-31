void k_p4wq_submit(struct k_p4wq *queue, struct k_p4wq_work *item)
{
	k_spinlock_key_t k = k_spin_lock(&queue->lock);

	/* Input is a delta time from now (to match
	 * k_thread_deadline_set()), but we store and use the absolute
	 * cycle count.
	 */
	item->deadline += k_cycle_get_32();

	/* Resubmission from within handler?  Remove from active list */
	if (item->thread == _current) {
		sys_dlist_remove(&item->dlnode);
		thread_set_requeued(_current);
		item->thread = NULL;
	} else {
		k_sem_init(&item->done_sem, 0, 1);
	}
	__ASSERT_NO_MSG(item->thread == NULL);

	rb_insert(&queue->queue, &item->rbnode);
	item->queue = queue;

	/* If there were other items already ahead of it in the queue,
	 * then we don't need to revisit active thread state and can
	 * return.
	 */
	if (rb_get_max(&queue->queue) != &item->rbnode) {
		goto out;
	}

	/* Check the list of active (running or preempted) items, if
	 * there are at least an "active target" of those that are
	 * higher priority than the new item, then no one needs to be
	 * preempted and we can return.
	 */
	struct k_p4wq_work *wi;
	uint32_t n_beaten_by = 0, active_target = arch_num_cpus();

	SYS_DLIST_FOR_EACH_CONTAINER(&queue->active, wi, dlnode) {
		/*
		 * item_lessthan(a, b) == true means a has lower priority than b
		 * !item_lessthan(a, b) counts all work items with higher or
		 * equal priority
		 */
		if (!item_lessthan(wi, item)) {
			n_beaten_by++;
		}
	}

	if (n_beaten_by >= active_target) {
		/* Too many already have higher priority, not preempting */
		goto out;
	}

	/* Grab a thread, set its priority and queue it.  If there are
	 * no threads available to unpend, this is a soft runtime
	 * error: we are breaking our promise about run order.
	 * Complain.
	 */
	struct k_thread *th = z_unpend_first_thread(&queue->waitq);

	if (th == NULL) {
		LOG_WRN("Out of worker threads, priority guarantee violated");
		goto out;
	}

	set_prio(th, item);
	z_ready_thread(th);
	z_reschedule(&queue->lock, k);

	return;

out:
	k_spin_unlock(&queue->lock, k);
}