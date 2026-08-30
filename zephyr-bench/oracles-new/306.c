void *mem_attr_heap_alloc(uint32_t attr, size_t bytes)
{
	return sys_multi_heap_alloc(&mah_data.multi_heap,
				    (void *)(long) attr, bytes);
}