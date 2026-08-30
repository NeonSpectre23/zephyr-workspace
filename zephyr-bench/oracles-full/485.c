void sys_multi_mem_blocks_add_allocator(sys_multi_mem_blocks_t *group,
					sys_mem_blocks_t *alloc)
{
	__ASSERT_NO_MSG(group->num_allocators < ARRAY_SIZE(group->allocators));

	group->allocators[group->num_allocators++] = alloc;
}