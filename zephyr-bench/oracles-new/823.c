void *sys_heap_aligned_realloc(struct sys_heap *heap, void *ptr,
			       size_t align, size_t bytes)
{
	/* special realloc semantics */
	if (ptr == NULL) {
		return sys_heap_aligned_alloc(heap, align, bytes);
	}
	if (bytes == 0) {
		sys_heap_free(heap, ptr);
		return NULL;
	}

	__ASSERT((align & (align - 1)) == 0, "align must be a power of 2");

	if ((align == 0 || ((uintptr_t)ptr & (align - 1)) == 0) &&
	    inplace_realloc(heap, ptr, bytes)) {
		return ptr;
	}

	/*
	 * Either ptr is not sufficiently aligned for in-place realloc or
	 * in-place realloc was not possible: fallback to allocate and copy.
	 */
	void *ptr2 = sys_heap_aligned_alloc(heap, align, bytes);

	if (ptr2 != NULL) {
		size_t prev_size = sys_heap_usable_size(heap, ptr);

		memcpy(ptr2, ptr, min(prev_size, bytes));
		sys_heap_free(heap, ptr);
	}
	return ptr2;
}

