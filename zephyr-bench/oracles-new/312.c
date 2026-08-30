void *min_heap_find(struct min_heap *heap, min_heap_eq_t eq,
		    const void *other, size_t *out_id)
{
	void *element;

	for (size_t i = 0; i < heap->size; ++i) {

		element = min_heap_get_element(heap, i);
		if (eq(element, other)) {
			if (out_id) {
				*out_id = i;
			}
			return element;
		}
	}

	return NULL;
}