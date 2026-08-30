int malloc_runtime_stats_get(struct sys_memory_stats *stats)
{
	int ret;

	malloc_lock();

	ret = sys_heap_runtime_stats_get(&z_malloc_heap, stats);

	malloc_unlock();

	return ret;
}