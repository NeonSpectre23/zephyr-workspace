int sys_mem_blocks_free_contiguous(sys_mem_blocks_t *mem_block, void *block, size_t count)
{
	int ret = 0;

	__ASSERT_NO_MSG(mem_block != NULL);
	__ASSERT_NO_MSG(mem_block->bitmap != NULL);
	__ASSERT_NO_MSG(mem_block->buffer != NULL);

	if (count == 0) {
		/* Nothing to be freed. */
		goto out;
	}

	if (count > mem_block->info.num_blocks) {
		ret = -EINVAL;
		goto out;
	}

	ret = free_blocks(mem_block, block, count);

	if (ret != 0) {
		goto out;
	}
#ifdef CONFIG_SYS_MEM_BLOCKS_LISTENER
	heap_listener_notify_free(HEAP_ID_FROM_POINTER(mem_block),
			block, count << mem_block->info.blk_sz_shift);
#endif

out:
	return ret;
}