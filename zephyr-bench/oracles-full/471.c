void sys_heap_stress(void *(*alloc_fn)(void *arg, size_t bytes),
		     void (*free_fn)(void *arg, void *p),
		     void *arg, size_t total_bytes,
		     uint32_t op_count,
		     void *scratch_mem, size_t scratch_bytes,
		     int target_percent,
		     struct z_heap_stress_result *result)
{
	struct z_heap_stress_rec sr = {
	       .alloc_fn = alloc_fn,
	       .free_fn = free_fn,
	       .arg = arg,
	       .total_bytes = total_bytes,
	       .blocks = scratch_mem,
	       .nblocks = scratch_bytes / sizeof(struct z_heap_stress_block),
	       .target_percent = target_percent,
	};

	*result = (struct z_heap_stress_result) {0};

	for (uint32_t i = 0; i < op_count; i++) {
		if (rand_alloc_choice(&sr)) {
			size_t sz = rand_alloc_size(&sr);
			void *p = sr.alloc_fn(sr.arg, sz);

			result->total_allocs++;
			if (p != NULL) {
				result->successful_allocs++;
				sr.blocks[sr.blocks_alloced].ptr = p;
				sr.blocks[sr.blocks_alloced].sz = sz;
				sr.blocks_alloced++;
				sr.bytes_alloced += sz;
			}
		} else {
			int b = rand_free_choice(&sr);
			void *p = sr.blocks[b].ptr;
			size_t sz = sr.blocks[b].sz;

			result->total_frees++;
			sr.blocks[b] = sr.blocks[sr.blocks_alloced - 1];
			sr.blocks_alloced--;
			sr.bytes_alloced -= sz;
			sr.free_fn(sr.arg, p);
		}
		result->accumulated_in_use_bytes += sr.bytes_alloced;
	}
}