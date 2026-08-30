void *sys_heap_realloc(struct sys_heap *heap, void *ptr, size_t bytes)
{
	/* special realloc semantics */
	if (ptr == NULL) {
		return sys_heap_alloc(heap, bytes);
	}
	if (bytes == 0) {
		sys_heap_free(heap, ptr);
		return NULL;
	}

	if (inplace_realloc(heap, ptr, bytes)) {
		return ptr;
	}

	/* In-place realloc was not possible: fallback to allocate and copy. */
	void *ptr2 = sys_heap_alloc(heap, bytes);

	if (ptr2 != NULL) {
		size_t prev_size = sys_heap_usable_size(heap, ptr);

		memcpy(ptr2, ptr, min(prev_size, bytes));
		sys_heap_free(heap, ptr);
	}
	return ptr2;
}

