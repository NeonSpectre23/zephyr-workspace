void pm_notifier_register(struct pm_notifier *notifier)
{
	k_spinlock_key_t pm_notifier_key = k_spin_lock(&pm_notifier_lock);

	sys_slist_append(&pm_notifiers, &notifier->_node);
	k_spin_unlock(&pm_notifier_lock, pm_notifier_key);
}