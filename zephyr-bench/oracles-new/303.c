int mem_attr_get_region_index_by_name(const char *target_name)
{
	const struct mem_attr_region_t *regions;
	size_t num_regions;

	num_regions = mem_attr_get_regions(&regions);

	for (int i = 0; i < num_regions; ++i) {
		if (regions[i].dt_name != NULL &&
		    strcmp(regions[i].dt_name, target_name) == 0) {
			return i;
		}
	}

	return -ENOENT;
}