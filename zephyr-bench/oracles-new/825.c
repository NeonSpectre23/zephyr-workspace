void sys_heap_free(struct sys_heap *heap, void *mem)
{
	if (mem == NULL) {
		return; /* ISO C free() semantics */
	}
	struct z_heap *h = heap->heap;
	chunkid_t c = mem_to_chunkid(h, mem);

	if (SYS_HEAP_HARDENING_BASIC && !chunk_used(h, c)) {
		LOG_ERR("heap corruption (double free?) at %p", mem);
		k_panic();
	}

	/*
	 * Header fields are ordered as LEFT_SIZE then SIZE_AND_USED.
	 * This places SIZE_AND_USED immediately before the user data,
	 * and LEFT_SIZE of the following chunk immediately after:
	 *
	 *       chunk c              right_chunk(c)
	 *   +-----------+---------+-----------+---------+
	 *   |  SIZE (1) | data    | L_SIZE (2)|  SIZE   |
	 *   +-----------+---------+-----------+---------+
	 *
	 * A buffer overflow from c's data corrupts field (2).
	 * Checking left(right(c)) == c catches this because
	 * right(c) reads field (1) and left() reads field (2):
	 * the two fields straddle the data buffer, so the
	 * round-trip fails if either side was corrupted.
	 */
	if (SYS_HEAP_HARDENING_BASIC &&
	    left_chunk(h, right_chunk(h, c)) != c) {
		LOG_ERR("heap corruption (buffer overflow?) at %p", mem);
		k_panic();
	}

	/*
	 * The above check validated SIZE_AND_USED using a field from
	 * the next chunk.  Now validate LEFT_SIZE: if it was corrupted
	 * (by an overflow from the left neighbor or an underflow from
	 * our own buffer) then right(left(c)) won't return to c.
	 */
	if (SYS_HEAP_HARDENING_MODERATE &&
	    right_chunk(h, left_chunk(h, c)) != c) {
		LOG_ERR("heap corruption (left neighbor?) at %p", mem);
		k_panic();
	}

	if (SYS_HEAP_HARDENING_FULL) {
		verify_chunk_canary(h, c, mem);
		poison_chunk_canary(h, c);
	}

	if (SYS_HEAP_HARDENING_EXTREME && !z_heap_full_check(h)) {
		LOG_ERR("heap validation failed");
		k_panic();
	}

	set_chunk_used(h, c, false);
#ifdef CONFIG_SYS_HEAP_RUNTIME_STATS
	h->allocated_bytes -= chunk_usable_bytes(h, c);
#endif

#ifdef CONFIG_SYS_HEAP_LISTENER
	heap_listener_notify_free(HEAP_ID_FROM_POINTER(heap), mem,
				  chunk_usable_bytes(h, c) - mem_align_gap(h, mem));
#endif

	free_chunk(h, c);
}

