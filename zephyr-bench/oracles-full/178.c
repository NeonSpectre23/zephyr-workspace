bool min_heap_pop(struct min_heap *heap, void *out_buf)
{
	return min_heap_remove(heap, 0, out_buf);
}