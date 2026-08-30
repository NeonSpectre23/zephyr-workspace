int sys_mem_blocks_alloc_contiguous(sys_mem_blocks_t *mem_block, size_t count,
				    void **out_block)
{
	int ret = 0;

	__ASSERT_NO_MSG(mem_block != NULL);
	__ASSERT_NO_MSG(out_block != NULL);

	if (count == 0) {
		/* Nothing to allocate */
		*out_block = NULL;
		goto out;
	}

	if (count > mem_block->info.num_blocks) {
		/* Definitely not enough blocks to be allocated */
		ret = -ENOMEM;
		goto out;
	}

	void *ptr = alloc_blocks(mem_block, count);

	if (ptr == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	*out_block = ptr;
#ifdef CONFIG_SYS_MEM_BLOCKS_LISTENER
	heap_listener_notify_alloc(HEAP_ID_FROM_POINTER(mem_block),
				   ptr, count << mem_block->info.blk_sz_shift);
#endif

out:
	return ret;
}