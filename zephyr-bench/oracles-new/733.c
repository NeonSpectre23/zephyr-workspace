void *shared_multi_heap_alloc(enum shared_multi_heap_attr attr, size_t bytes)
{
	if (attr >= MAX_SHARED_MULTI_HEAP_ATTR) {
		return NULL;
	}

	return sys_multi_heap_alloc(&shared_multi_heap, (void *)(long) attr, bytes);
}