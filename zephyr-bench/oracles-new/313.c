void *min_heap_peek(const struct min_heap *heap)
{
	if (heap->size == 0) {
		return NULL;
	}

	return min_heap_get_element(heap, 0);
}