void heap_listener_unregister(struct heap_listener *listener)
{
	k_spinlock_key_t key = k_spin_lock(&heap_listener_lock);

	sys_slist_find_and_remove(&heap_listener_list, &listener->node);

	k_spin_unlock(&heap_listener_lock, key);
}