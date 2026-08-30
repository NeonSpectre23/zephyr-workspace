int k_mem_domain_add_thread(struct k_mem_domain *domain, k_tid_t thread)
{
	int ret = 0;
	k_spinlock_key_t key;

	key = k_spin_lock(&z_mem_domain_lock);
	if (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		if (ret == 0) {
			ret = add_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_lock, key);

	return ret;
}