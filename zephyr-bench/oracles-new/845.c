void sys_multi_mem_blocks_init(sys_multi_mem_blocks_t *group,
			       sys_multi_mem_blocks_choice_fn_t choice_fn)
{
	group->num_allocators = 0;
	group->choice_fn = choice_fn;
}