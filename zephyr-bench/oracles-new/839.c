void *sys_multi_heap_alloc(struct sys_multi_heap *mheap, void *cfg, size_t bytes)
{
	return mheap->choice(mheap, cfg, 0, bytes);
}