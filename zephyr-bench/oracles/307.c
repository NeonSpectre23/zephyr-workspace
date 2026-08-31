void pm_policy_latency_changed_subscribe(struct pm_policy_latency_subscription *req,
					 pm_policy_latency_changed_cb_t cb)
{
	k_spinlock_key_t key = k_spin_lock(&latency_lock);

	if (cb == NULL) {
		req->cb = NULL;
		(void)sys_slist_find_and_remove(&latency_subs, &req->node);
		k_spin_unlock(&latency_lock, key);
		return;
	}

	req->cb = cb;
	sys_slist_append(&latency_subs, &req->node);

	k_spin_unlock(&latency_lock, key);
}