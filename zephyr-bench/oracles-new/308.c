const struct mem_attr_region_t *mem_attr_heap_get_region(void *addr)
{
	const struct sys_multi_heap_rec *heap_rec;

	heap_rec = sys_multi_heap_get_heap(&mah_data.multi_heap, addr);

	return (const struct mem_attr_region_t *) heap_rec->user_data;
}