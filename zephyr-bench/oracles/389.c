int sys_mem_blocks_runtime_stats_get(sys_mem_blocks_t *mem_block,
				     struct sys_memory_stats *stats)
{
	if ((mem_block == NULL) || (stats == NULL)) {
		return -EINVAL;
	}

	stats->allocated_bytes = mem_block->info.used_blocks <<
				 mem_block->info.blk_sz_shift;
	stats->free_bytes = (mem_block->info.num_blocks <<
			     mem_block->info.blk_sz_shift) -
			    stats->allocated_bytes;
	stats->max_allocated_bytes = mem_block->info.max_used_blocks <<
				     mem_block->info.blk_sz_shift;

	return 0;
}