void *sys_heap_alloc(struct sys_heap *heap, size_t bytes)
{
	struct z_heap *h = heap->heap;
	void *mem;

	if (bytes == 0U) {
		return NULL;
	}

	chunksz_t chunk_sz = bytes_to_chunksz(h, bytes, 0);
	chunkid_t c = alloc_chunk(h, chunk_sz);

	if (c == 0U) {
		return NULL;
	}

	/* Split off remainder if any */
	if (chunk_size(h, c) > chunk_sz) {
		split_chunks(h, c, c + chunk_sz);
		free_list_add(h, c + chunk_sz);
	}

	set_chunk_used(h, c, true);
	if (SYS_HEAP_HARDENING_FULL) {
		set_chunk_canary(h, c);
	}

	mem = chunk_mem(h, c);

#ifdef CONFIG_SYS_HEAP_RUNTIME_STATS
	increase_allocated_bytes(h, chunk_usable_bytes(h, c));
#endif

#ifdef CONFIG_SYS_HEAP_LISTENER
	heap_listener_notify_alloc(HEAP_ID_FROM_POINTER(heap), mem,
				   chunk_usable_bytes(h, c));
#endif

	IF_ENABLED(CONFIG_MSAN, (__msan_allocated_memory(mem, bytes)));
	return mem;
}