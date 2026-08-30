void sys_heap_init(struct sys_heap *heap, void *mem, size_t bytes)
{
	IF_ENABLED(CONFIG_MSAN, (__sanitizer_dtor_callback(mem, bytes)));

	if (IS_ENABLED(CONFIG_SYS_HEAP_SMALL_ONLY)) {
		/* Must fit in a 15 bit count of HUNK_UNIT */
		__ASSERT(bytes / CHUNK_UNIT <= 0x7fffU, "heap size is too big");
	} else {
		/* Must fit in a 31 bit count of HUNK_UNIT */
		__ASSERT(bytes / CHUNK_UNIT <= 0x7fffffffU, "heap size is too big");
	}

	/* Reserve the end marker chunk's header */
	__ASSERT(bytes > heap_footer_bytes(bytes), "heap size is too small");
	bytes -= heap_footer_bytes(bytes);

	/* Round the start up, the end down */
	uintptr_t addr = ROUND_UP(mem, CHUNK_UNIT);
	uintptr_t end = ROUND_DOWN((uint8_t *)mem + bytes, CHUNK_UNIT);
	chunksz_t heap_sz = (end - addr) / CHUNK_UNIT;

	CHECK(end > addr);
	__ASSERT(heap_sz > chunksz(sizeof(struct z_heap)), "heap size is too small");

	struct z_heap *h = (struct z_heap *)addr;
	heap->heap = h;
	h->end_chunk = heap_sz;
	h->avail_buckets = 0;

#ifdef CONFIG_SYS_HEAP_RUNTIME_STATS
	h->free_bytes = 0;
	h->allocated_bytes = 0;
	h->max_allocated_bytes = 0;
#endif

#if CONFIG_SYS_HEAP_ARRAY_SIZE
	sys_heap_array_save(heap);
#endif

	int nb_buckets = bucket_idx(h, heap_sz) + 1;
	chunksz_t chunk0_size = chunksz(sizeof(struct z_heap) +
				     nb_buckets * sizeof(struct z_heap_bucket)) +
				CHUNK_TRAILER_SIZE;

	__ASSERT(chunk0_size + min_chunk_size(h) <= heap_sz, "heap size is too small");

	for (int i = 0; i < nb_buckets; i++) {
		h->buckets[i].next = 0;
	}

	/* chunk containing our struct z_heap */
	set_chunk_size(h, 0, chunk0_size);
	set_left_chunk_size(h, 0, 0);
	set_chunk_used(h, 0, true);
	if (SYS_HEAP_HARDENING_FULL) {
		set_chunk_canary(h, 0);
	}

	/* chunk containing the free heap */
	set_chunk_size(h, chunk0_size, heap_sz - chunk0_size);
	set_left_chunk_size(h, chunk0_size, chunk0_size);

	/* the end marker chunk */
	set_chunk_size(h, heap_sz, 0);
	set_left_chunk_size(h, heap_sz, heap_sz - chunk0_size);
	set_chunk_used(h, heap_sz, true);

	free_list_add(h, chunk0_size);
}
