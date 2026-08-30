bool min_heap_remove(struct min_heap *heap, size_t id, void *out_buf)
{
	if (id >= heap->size) {
		return false;
	}

	void *removed = min_heap_get_element(heap, id);

	memcpy(out_buf, removed, heap->elem_size);
	heap->size--;
	if (id != heap->size) {
		void *last = min_heap_get_element(heap, heap->size);

		memcpy(removed, last, heap->elem_size);
		heapify_down(heap, id);
		heapify_up(heap, id);
	}

	return true;
}