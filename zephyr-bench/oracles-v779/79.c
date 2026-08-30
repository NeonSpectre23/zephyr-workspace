void heap_listener_register(struct heap_listener *listener)
{
	k_spinlock_key_t key = k_spin_lock(&heap_listener_lock);

	sys_slist_append(&heap_listener_list, &listener->node);

	k_spin_unlock(&heap_listener_lock, key);
}