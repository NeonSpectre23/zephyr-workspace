void pm_policy_latency_request_add(struct pm_policy_latency_request *req,
				   uint32_t value_us)
{
	req->value_us = value_us;

	k_spinlock_key_t key = k_spin_lock(&latency_lock);

	sys_slist_append(&latency_reqs, &req->node);
	update_max_latency();

	k_spin_unlock(&latency_lock, key);
}