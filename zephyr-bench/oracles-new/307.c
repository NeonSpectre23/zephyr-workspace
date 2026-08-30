void mem_attr_heap_free(void *block)
{
	sys_multi_heap_free(&mah_data.multi_heap, block);
}