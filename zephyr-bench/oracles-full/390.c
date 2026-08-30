void pm_policy_latency_request_update(struct pm_policy_latency_request *req,
				      uint32_t value_us)
{
	k_spinlock_key_t key = k_spin_lock(&latency_lock);

	req->value_us = value_us;
	update_max_latency();

	k_spin_unlock(&latency_lock, key);
}