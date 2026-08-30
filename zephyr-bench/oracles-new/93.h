static inline bool min_heap_is_empty(struct min_heap *heap)
{
	__ASSERT_NO_MSG(heap != NULL);

	return (heap->size == 0);
}