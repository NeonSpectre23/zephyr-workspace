int mem_attr_heap_pool_init(void)
{
	const struct mem_attr_region_t *regions;
	static atomic_t state;
	size_t num_regions;

	if (!atomic_cas(&state, 0, 1)) {
		return -EALREADY;
	}

	sys_multi_heap_init(&mah_data.multi_heap, mah_choice);

	num_regions = mem_attr_get_regions(&regions);

	for (size_t idx = 0; idx < num_regions; idx++) {
		uint32_t sw_attr;

		sw_attr = DT_MEM_SW_ATTR_GET(regions[idx].dt_attr);

		/* No SW attribute is present */
		if (!sw_attr) {
			continue;
		}

		if (ma_heap_add(&regions[idx], sw_attr)) {
			return -ENOMEM;
		}
	}

	return 0;
}