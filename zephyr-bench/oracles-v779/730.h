static inline size_t sys_hashmap_size(const struct sys_hashmap *map)
{
	return map->data->size;
}