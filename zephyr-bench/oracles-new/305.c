void *mem_attr_heap_aligned_alloc(uint32_t attr, size_t align, size_t bytes)
{
	return sys_multi_heap_aligned_alloc(&mah_data.multi_heap,
					    (void *)(long) attr, align, bytes);
}