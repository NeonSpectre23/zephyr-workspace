bool sys_heap_validate(struct sys_heap *heap)
{
	struct z_heap *h = heap->heap;

	if (!z_heap_full_check(h)) {
		return false;
	}

#ifdef CONFIG_SYS_HEAP_RUNTIME_STATS
	/*
	 * Validate sys_heap_runtime_stats_get API.
	 * Iterate all chunks in sys_heap to get total allocated bytes and
	 * free bytes, then compare with the results of
	 * sys_heap_runtime_stats_get function.
	 */
	size_t allocated_bytes, free_bytes;
	struct sys_memory_stats stat;

	get_alloc_info(h, &allocated_bytes, &free_bytes);
	sys_heap_runtime_stats_get(heap, &stat);
	if ((stat.allocated_bytes != allocated_bytes) ||
	    (stat.free_bytes != free_bytes)) {
		return false;
	}
#endif

	return true;
}