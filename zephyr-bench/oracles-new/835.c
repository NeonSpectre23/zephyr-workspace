int sys_mem_blocks_get(sys_mem_blocks_t *mem_block, void *in_block, size_t count)
{
	int ret = 0;
	int offset;

	__ASSERT_NO_MSG(mem_block != NULL);
	__ASSERT_NO_MSG(mem_block->bitmap != NULL);
	__ASSERT_NO_MSG(mem_block->buffer != NULL);

	if (count == 0) {
		/* Nothing to allocate */
		goto out;
	}

	offset = ((uint8_t *)in_block - mem_block->buffer) >>
		 mem_block->info.blk_sz_shift;

	if (offset + count > mem_block->info.num_blocks) {
		/* Definitely not enough blocks to be allocated */
		ret = -ENOMEM;
		goto out;
	}

#ifdef CONFIG_SYS_MEM_BLOCKS_RUNTIME_STATS
	k_spinlock_key_t  key = k_spin_lock(&mem_block->lock);
#endif

	ret = sys_bitarray_test_and_set_region(mem_block->bitmap, count,
					       offset, true);

	if (ret != 0) {
#ifdef CONFIG_SYS_MEM_BLOCKS_RUNTIME_STATS
		k_spin_unlock(&mem_block->lock, key);
#endif
		ret = -ENOMEM;
		goto out;
	}

#ifdef CONFIG_SYS_MEM_BLOCKS_RUNTIME_STATS
	mem_block->info.used_blocks += (uint32_t)count;

	if (mem_block->info.max_used_blocks < mem_block->info.used_blocks) {
		mem_block->info.max_used_blocks = mem_block->info.used_blocks;
	}

	k_spin_unlock(&mem_block->lock, key);
#endif

#ifdef CONFIG_SYS_MEM_BLOCKS_LISTENER
	heap_listener_notify_alloc(HEAP_ID_FROM_POINTER(mem_block),
			in_block, count << mem_block->info.blk_sz_shift);
#endif

out:
	return ret;
}