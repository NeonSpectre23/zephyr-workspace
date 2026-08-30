void *sys_heap_aligned_alloc(struct sys_heap *heap, size_t align, size_t bytes)
{
	struct z_heap *h = heap->heap;
	size_t gap, rew;

	/*
	 * Split align and rewind values (if any).
	 * We allow for one bit of rewind in addition to the alignment
	 * value to efficiently accommodate z_alloc_helper().
	 * So if e.g. align = 0x28 (32 | 8) this means we align to a 32-byte
	 * boundary and then rewind 8 bytes.
	 */
	rew = align & -align;
	if (align != rew) {
		align -= rew;
		gap = min(rew, chunk_header_bytes(h));
	} else {
		if (align <= chunk_header_bytes(h)) {
			return sys_heap_alloc(heap, bytes);
		}
		rew = 0;
		gap = chunk_header_bytes(h);
	}
	__ASSERT((align & (align - 1)) == 0, "align must be a power of 2");

	if (bytes == 0) {
		return NULL;
	}

	/*
	 * Find a free block that is guaranteed to fit.
	 * We over-allocate to account for alignment and then free
	 * the extra allocations afterwards.
	 */
	chunksz_t padded_sz = bytes_to_chunksz(h, bytes, align - gap);
	chunkid_t c0 = alloc_chunk(h, padded_sz);

	if (c0 == 0) {
		return NULL;
	}
	uint8_t *mem = chunk_mem(h, c0);

	/* Align allocated memory */
	mem = (uint8_t *) ROUND_UP(mem + rew, align) - rew;
	chunk_unit_t *end = (chunk_unit_t *) ROUND_UP(mem + bytes, CHUNK_UNIT);

	/* Get corresponding chunks */
	chunkid_t c = mem_to_chunkid(h, mem);
	chunkid_t c_end = end - chunk_buf(h) + CHUNK_TRAILER_SIZE;
	CHECK(c >= c0 && c  < c_end && c_end <= c0 + padded_sz);

	/* Split and free unused prefix */
	if (c > c0) {
		split_chunks(h, c0, c);
		free_list_add(h, c0);
	}

	/* Split and free unused suffix */
	if (right_chunk(h, c) > c_end) {
		split_chunks(h, c, c_end);
		free_list_add(h, c_end);
	}

	set_chunk_used(h, c, true);
	if (SYS_HEAP_HARDENING_FULL) {
		set_chunk_canary(h, c);
	}

#ifdef CONFIG_SYS_HEAP_RUNTIME_STATS
	increase_allocated_bytes(h, chunk_usable_bytes(h, c));
#endif

#ifdef CONFIG_SYS_HEAP_LISTENER
	heap_listener_notify_alloc(HEAP_ID_FROM_POINTER(heap), mem,
				   chunk_usable_bytes(h, c) - mem_align_gap(h, mem));
#endif

	IF_ENABLED(CONFIG_MSAN, (__msan_allocated_memory(mem, bytes)));
	return mem;
}