static inline bool sys_hashmap_is_empty(const struct sys_hashmap *map)
{
	return map->data->size == 0;
}