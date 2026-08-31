int min_heap_push(struct min_heap *heap, const void *item)
{
	if (heap->size >= heap->capacity) {
		return -ENOMEM;
	}

	void *dest = min_heap_get_element(heap, heap->size);

	memcpy(dest, item, heap->elem_size);
	heapify_up(heap, heap->size);
	heap->size++;

	return 0;
}