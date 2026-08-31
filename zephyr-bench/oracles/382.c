size_t sys_heap_usable_size(struct sys_heap *heap, void *mem)
{
	struct z_heap *h = heap->heap;
	chunkid_t c = mem_to_chunkid(h, mem);

	if (SYS_HEAP_HARDENING_FULL) {
		verify_chunk_canary(h, c, mem);
	}

	return chunk_usable_bytes(h, c) - mem_align_gap(h, mem);
}