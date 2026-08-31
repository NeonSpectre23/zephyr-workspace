size_t mem_attr_get_regions(const struct mem_attr_region_t **region)
{
	*region = mem_attr_region;

	return ARRAY_SIZE(mem_attr_region);
}