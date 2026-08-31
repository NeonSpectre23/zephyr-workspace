void sys_multi_heap_init(struct sys_multi_heap *heap, sys_multi_heap_fn_t choice_fn)
{
	heap->nheaps = 0;
	heap->choice = choice_fn;
}