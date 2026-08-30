int sys_mem_blocks_runtime_stats_reset_max(sys_mem_blocks_t *mem_block)
{
	if (mem_block == NULL) {
		return -EINVAL;
	}

	mem_block->info.max_used_blocks = mem_block->info.used_blocks;

	return 0;
}