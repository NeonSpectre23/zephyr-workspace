void sys_multi_heap_free(struct sys_multi_heap *mheap, void *block)
{
	const struct sys_multi_heap_rec *heap;

	heap = sys_multi_heap_get_heap(mheap, block);

	if (heap != NULL) {
		sys_heap_free(heap->heap, block);
	}
}