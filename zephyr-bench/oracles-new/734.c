int shared_multi_heap_pool_init(void)
{
	static atomic_t state;

	if (!atomic_cas(&state, 0, 1)) {
		return -EALREADY;
	}

	sys_multi_heap_init(&shared_multi_heap, smh_choice);

	atomic_set(&state, 1);

	return 0;
}