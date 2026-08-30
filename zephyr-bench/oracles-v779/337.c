void pm_policy_latency_changed_unsubscribe(struct pm_policy_latency_subscription *req)
{
	k_spinlock_key_t key = k_spin_lock(&latency_lock);

	(void)sys_slist_find_and_remove(&latency_subs, &req->node);

	k_spin_unlock(&latency_lock, key);
}