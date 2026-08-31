int sys_mem_blocks_free(sys_mem_blocks_t *mem_block, size_t count,
			void **in_blocks)
{
	int ret = 0;
	int i;

	__ASSERT_NO_MSG(mem_block != NULL);
	__ASSERT_NO_MSG(in_blocks != NULL);
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

	for (i = 0; i < count; i++) {
		void *ptr = in_blocks[i];

		int r = free_blocks(mem_block, ptr, 1);

		if (r != 0) {
			ret = r;
		}
#ifdef CONFIG_SYS_MEM_BLOCKS_LISTENER
		else {
			/*
			 * Since we do not keep track of failed free ops,
			 * we need to notify free one-by-one, instead of
			 * notifying at the end of function.
			 */
			heap_listener_notify_free(HEAP_ID_FROM_POINTER(mem_block),
						  ptr, BIT(mem_block->info.blk_sz_shift));
		}
#endif
	}

out:
	return ret;
}