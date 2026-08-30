int sys_multi_mem_blocks_alloc(sys_multi_mem_blocks_t *group,
			       void *cfg, size_t count,
			       void **out_blocks,
			       size_t *blk_size)
{
	sys_mem_blocks_t *allocator;
	int ret = 0;

	__ASSERT_NO_MSG(group != NULL);
	__ASSERT_NO_MSG(out_blocks != NULL);

	if (count == 0) {
		if (blk_size != NULL) {
			*blk_size = 0;
		}
		goto out;
	}

	allocator = group->choice_fn(group, cfg);
	if (allocator == NULL) {
		ret = -EINVAL;
		goto out;
	}

	if (count > allocator->info.num_blocks) {
		ret = -ENOMEM;
		goto out;
	}

	ret = sys_mem_blocks_alloc(allocator, count, out_blocks);

	if ((ret == 0) && (blk_size != NULL)) {
		*blk_size = BIT(allocator->info.blk_sz_shift);
	}

out:
	return ret;
}