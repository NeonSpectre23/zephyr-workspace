int sys_mem_blocks_alloc(sys_mem_blocks_t *mem_block, size_t count,
			 void **out_blocks)
{
	int ret = 0;
	int i;

	__ASSERT_NO_MSG(mem_block != NULL);
	__ASSERT_NO_MSG(out_blocks != NULL);
	__ASSERT_NO_MSG(mem_block->bitmap != NULL);
	__ASSERT_NO_MSG(mem_block->buffer != NULL);

	if (count == 0) {
		/* Nothing to allocate */
		goto out;
	}

	if (count > mem_block->info.num_blocks) {
		/* Definitely not enough blocks to be allocated */
		ret = -ENOMEM;
		goto out;
	}

	for (i = 0; i < count; i++) {
		void *ptr = alloc_blocks(mem_block, 1);

		if (ptr == NULL) {
			break;
		}

		out_blocks[i] = ptr;

#ifdef CONFIG_SYS_MEM_BLOCKS_LISTENER
		heap_listener_notify_alloc(HEAP_ID_FROM_POINTER(mem_block),
					   ptr,
					   BIT(mem_block->info.blk_sz_shift));
#endif
	}

	/* If error, free already allocated blocks. */
	if (i < count) {
		(void)sys_mem_blocks_free(mem_block, i, out_blocks);
		ret = -ENOMEM;
	}

out:
	return ret;
}