uint32_t rtio_work_req_used_count_get(void)
{
	return k_mem_slab_num_used_get(&rtio_work_items_slab);
}