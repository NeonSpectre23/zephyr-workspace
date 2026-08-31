int sys_multi_mem_blocks_free(sys_multi_mem_blocks_t *group,
			      size_t count, void **in_blocks)
{
	int i;
	int ret = 0;
	sys_mem_blocks_t *allocator = NULL;

	__ASSERT_NO_MSG(group != NULL);
	__ASSERT_NO_MSG(in_blocks != NULL);

	if (count == 0) {
		goto out;
	}

	for (i = 0; i < group->num_allocators; i++) {
		/*
		 * Find out which allocator the allocated blocks
		 * belong to.
		 */

		uint8_t *start, *end;
		sys_mem_blocks_t *one_alloc;

		one_alloc = group->allocators[i];
		start = one_alloc->buffer;
		end = start + (BIT(one_alloc->info.blk_sz_shift) *
			       one_alloc->info.num_blocks);

		if (((uint8_t *)in_blocks[0] >= start) &&
		    ((uint8_t *)in_blocks[0] < end)) {
			allocator = one_alloc;
			break;
		}
	}

	if (allocator != NULL) {
		ret = sys_mem_blocks_free(allocator, count, in_blocks);
	} else {
		ret = -EINVAL;
	}

out:
	return ret;
}