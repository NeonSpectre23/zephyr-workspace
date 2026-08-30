void pm_policy_latency_request_remove(struct pm_policy_latency_request *req)
{
	k_spinlock_key_t key = k_spin_lock(&latency_lock);

	(void)sys_slist_find_and_remove(&latency_reqs, &req->node);
	update_max_latency();

	k_spin_unlock(&latency_lock, key);
}