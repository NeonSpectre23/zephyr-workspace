void k_free(void *ptr)
{
	struct k_heap **heap_ref;

	if (ptr != NULL) {
		heap_ref = ptr;
		--heap_ref;
		ptr = heap_ref;

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_free, *heap_ref, heap_ref);

		k_heap_free(*heap_ref, ptr);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_free, *heap_ref, heap_ref);
	}
}